//  SuperTux - SuperMathTux feature
#include "supertux/math_question.hpp"

#include <algorithm>
#include <sstream>

#include "math/random.hpp"

static int pick_int(int lo, int hi_inclusive)
{
  // graphicsRandom.rand(u, v) is [u, v); gameRandom is for gameplay.
  if (hi_inclusive < lo)
    std::swap(lo, hi_inclusive);
  return gameRandom.rand(lo, hi_inclusive + 1);
}

static std::vector<int> make_distractors(int correct, int spread, int count, bool allow_negative)
{
  std::vector<int> out;
  out.reserve(count);
  // Deterministic offsets, shuffled, to avoid duplicates.
  std::vector<int> offsets;
  for (int d = 1; d <= spread + 2 && (int)offsets.size() < 12; ++d)
  {
    offsets.push_back(d);
    offsets.push_back(-d);
  }
  offsets.push_back(10);
  offsets.push_back(-10);
  offsets.push_back(2);
  offsets.push_back(-2);

  // Shuffle offsets using gameRandom.
  for (size_t i = offsets.size(); i > 1; --i)
  {
    size_t j = static_cast<size_t>(gameRandom.rand(0, static_cast<int>(i)));
    std::swap(offsets[i - 1], offsets[j]);
  }

  for (int off : offsets)
  {
    if ((int)out.size() >= count)
      break;
    int cand = correct + off;
    if (!allow_negative && cand < 0)
      continue;
    if (std::find(out.begin(), out.end(), cand) != out.end())
      continue;
    if (cand == correct)
      continue;
    out.push_back(cand);
  }
  // Fallback fill (upwards only, so it stays non-negative when required).
  int filler = correct + spread + 3;
  if (!allow_negative && filler < 0)
    filler = correct + 1;
  while ((int)out.size() < count)
  {
    if (!allow_negative && filler < 0)
    {
      ++filler;
      continue;
    }
    if (filler != correct && std::find(out.begin(), out.end(), filler) == out.end())
      out.push_back(filler);
    ++filler;
  }
  return out;
}

MathQuestion
MathQuestion::generate(int grade_level)
{
  if (grade_level < 1)
    grade_level = 1;
  if (grade_level > 6)
    grade_level = 6;

  int a = 0, b = 0, result = 0;
  char op = '+';

  if (grade_level <= 1)
  {
    // Grade 1: + and - with small numbers 0..10, non-negative results.
    op = (pick_int(0, 1) == 0) ? '+' : '-';
    a = pick_int(0, 10);
    b = pick_int(0, 10);
    if (op == '-' && b > a)
      std::swap(a, b);
    result = (op == '+') ? (a + b) : (a - b);
  }
  else if (grade_level == 2)
  {
    // Grade 2: + and - with larger numbers 0..100.
    op = (pick_int(0, 1) == 0) ? '+' : '-';
    a = pick_int(0, 100);
    b = pick_int(0, 100);
    if (op == '-' && b > a)
      std::swap(a, b);
    result = (op == '+') ? (a + b) : (a - b);
  }
  else if (grade_level == 3)
  {
    // Grade 3: +, -, and x.
    int kind = pick_int(0, 2);
    if (kind == 0)
    {
      op = '+';
      a = pick_int(0, 100);
      b = pick_int(0, 100);
      result = a + b;
    }
    else if (kind == 1)
    {
      op = '-';
      a = pick_int(0, 100);
      b = pick_int(0, 100);
      if (b > a)
        std::swap(a, b);
      result = a - b;
    }
    else
    {
      op = 'x';
      a = pick_int(2, 10);
      b = pick_int(2, 10);
      result = a * b;
    }
  }
  else
  {
    // Grade 4+: +, -, x, division. Ranges grow with grade.
    int max_add = (grade_level == 4) ? 100 : 1000;
    int max_mul = (grade_level == 4) ? 12 : ((grade_level == 5) ? 15 : 20);
    int kind = pick_int(0, 3);
    if (kind == 0)
    {
      op = '+';
      a = pick_int(0, max_add);
      b = pick_int(0, max_add);
      result = a + b;
    }
    else if (kind == 1)
    {
      op = '-';
      a = pick_int(0, max_add);
      b = pick_int(0, max_add);
      if (b > a)
        std::swap(a, b);
      result = a - b;
    }
    else if (kind == 2)
    {
      op = 'x';
      a = pick_int(2, max_mul);
      b = pick_int(2, max_mul);
      result = a * b;
    }
    else
    {
      // Exact division: pick divisor and quotient, then dividend = divisor * quotient.
      op = '/';
      b = pick_int(2, max_mul);
      int q = pick_int(2, max_mul);
      a = b * q;
      result = q;
    }
  }

  std::ostringstream qs;
  qs << "What is " << a << " " << op << " " << b << " ?";
  MathQuestion q;
  q.question_text = qs.str();
  q.correct_value = result;

  // Grades 1-4 must not show negative answers (correct results are already
  // kept non-negative via operand swapping above).
  const bool allow_negative = (grade_level > 4);
  auto distract = make_distractors(result, (grade_level <= 1) ? 3 : 10, 3, allow_negative);
  std::vector<int> all_values;
  all_values.push_back(result);
  all_values.insert(all_values.end(), distract.begin(), distract.end());
  // Shuffle answer order.
  for (size_t i = all_values.size(); i > 1; --i)
  {
    size_t j = static_cast<size_t>(gameRandom.rand(0, static_cast<int>(i)));
    std::swap(all_values[i - 1], all_values[j]);
  }
  for (size_t i = 0; i < all_values.size(); ++i)
  {
    q.answers.push_back(std::to_string(all_values[i]));
    if (all_values[i] == result)
      q.correct_index = static_cast<int>(i);
  }
  return q;
}
