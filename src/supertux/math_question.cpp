//  SuperTux - SuperMathTux feature
#include "supertux/math_question.hpp"

#include <algorithm>
#include <sstream>
#include <string>

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

// --- Word problems -------------------------------------------------------

struct PersonInfo
{
  const char* name;
  bool female; // true: she/her, false: he/him
};

static const PersonInfo NAMES[] = {
  {"Betty", true}, {"Alice", true}, {"Lucy", true},
  {"Emma", true}, {"Maria", true}, {"Sophie", true},
  {"Marcus", false}, {"Bobby", false}, {"Pete", false},
  {"Tom", false}, {"Jack", false}, {"Leo", false},
};

// Regular plurals only (plural = singular + "s").
static const char* ITEMS[] = {
  "apple", "marble", "pencil", "pen", "book", "ball", "sticker", "crayon",
};

static std::string subj_cap(bool female) { return female ? "She" : "He"; }
static std::string subj_low(bool female) { return female ? "she" : "he"; }
static std::string poss_cap(bool female) { return female ? "Her" : "His"; }
static std::string obj_low(bool female) { return female ? "her" : "him"; }

static std::string plural_item(const std::string& item) { return item + "s"; }

static std::string count_noun(const std::string& item, int n)
{
  return std::to_string(n) + " " + (n == 1 ? item : plural_item(item));
}

static const PersonInfo& pick_person()
{
  const int n = static_cast<int>(sizeof(NAMES) / sizeof(NAMES[0]));
  return NAMES[pick_int(0, n - 1)];
}

// Pick two distinct names (P1 != P2).
static void pick_two_distinct(const PersonInfo*& p1, const PersonInfo*& p2)
{
  const int n = static_cast<int>(sizeof(NAMES) / sizeof(NAMES[0]));
  int i1 = pick_int(0, n - 1);
  int i2 = pick_int(0, n - 2);
  if (i2 >= i1)
    ++i2;
  p1 = &NAMES[i1];
  p2 = &NAMES[i2];
}

static std::string pick_item()
{
  const int n = static_cast<int>(sizeof(ITEMS) / sizeof(ITEMS[0]));
  return ITEMS[pick_int(0, n - 1)];
}

// Subtraction story: P1 had n1, gave n2 to P2. Answer n1-n2.
static std::string build_give_away(const PersonInfo& p1, const PersonInfo& p2,
                                   const std::string& item, int n1, int n2)
{
  std::ostringstream ss;
  ss << p1.name << " had " << count_noun(item, n1) << ". "
     << subj_cap(p1.female) << " gave " << count_noun(item, n2)
     << " to " << p2.name << ". "
     << "How many " << plural_item(item) << " does " << subj_low(p1.female) << " have?";
  return ss.str();
}

// Addition story: P1 had n1, friend gave another n2. Answer n1+n2.
static std::string build_receive_gift(const PersonInfo& p1,
                                      const std::string& item, int n1, int n2)
{
  std::ostringstream ss;
  ss << p1.name << " had " << count_noun(item, n1) << ". "
     << poss_cap(p1.female) << " friend gave " << obj_low(p1.female)
     << " another " << count_noun(item, n2) << ". "
     << "How many does " << p1.name << " have?";
  return ss.str();
}

// Missing-subtrahend story: P1 had n1, now only has n2. Answer n1-n2 to P2.
static std::string build_missing_give(const PersonInfo& p1, const PersonInfo& p2,
                                      const std::string& item, int n1, int n2)
{
  std::ostringstream ss;
  ss << p1.name << " had " << count_noun(item, n1) << ". "
     << subj_cap(p1.female) << " gave some to " << p2.name << ". "
     << "Now " << p1.name << " only has " << count_noun(item, n2) << ". "
     << "How many " << plural_item(item) << " did " << p2.name << " get?";
  return ss.str();
}

// Multiplication story: P1 gets n1 items from each of n2 friends. Answer n1*n2.
static std::string build_mult_gifts(const PersonInfo& p1,
                                    const std::string& item, int n1, int n2)
{
  std::ostringstream ss;
  ss << p1.name << " gets " << count_noun(item, n1)
     << " from " << n2 << " friends. "
     << "How many " << plural_item(item) << " does " << subj_low(p1.female) << " have?";
  return ss.str();
}

// Division story: P1 has n1, shares equally among n2 friends. Answer n1/n2.
static std::string build_divide_share(const PersonInfo& p1,
                                      const std::string& item, int n1, int n2)
{
  std::ostringstream ss;
  ss << p1.name << " has " << count_noun(item, n1)
     << ", " << subj_low(p1.female)
     << " wants to divide and give an equal number to " << n2 << " friends. "
     << "How much does each friend get?";
  return ss.str();
}

// Try to render (op, a, b) as a word problem reusing the already-picked
// grade-appropriate numbers, so numeric ranges and exact-division guarantees
// come for free. Returns false when the numbers don't make a sensible story
// (e.g. giving 0 items), in which case the caller falls back to symbolic.
static bool try_build_word_problem(char op, int a, int b, std::string& out)
{
  if (op == '+')
  {
    // Need "had n1" and "another n2" to both be at least 1 to read sensibly.
    if (a < 1 || b < 1)
      return false;
    const PersonInfo& p1 = pick_person();
    out = build_receive_gift(p1, pick_item(), a, b);
    return true;
  }
  if (op == '-')
  {
    // Two subtraction flavors; try in random order for variety.
    const bool give_first = (pick_int(0, 1) == 0);
    for (int attempt = 0; attempt < 2; ++attempt)
    {
      const bool give_away = (attempt == 0) ? give_first : !give_first;
      if (give_away)
      {
        // "gave n2 to P2": n2 must be >= 1 (a >= b already guaranteed).
        if (a >= 1 && b >= 1)
        {
          const PersonInfo *p1 = nullptr, *p2 = nullptr;
          pick_two_distinct(p1, p2);
          out = build_give_away(*p1, *p2, pick_item(), a, b);
          return true;
        }
      }
      else
      {
        // "gave some ... now only has n2": need a > b so P2 got >= 1.
        if (a > b)
        {
          const PersonInfo *p1 = nullptr, *p2 = nullptr;
          pick_two_distinct(p1, p2);
          out = build_missing_give(*p1, *p2, pick_item(), a, b);
          return true;
        }
      }
    }
    return false;
  }
  if (op == 'x')
  {
    const PersonInfo& p1 = pick_person();
    out = build_mult_gifts(p1, pick_item(), a, b);
    return true;
  }
  if (op == '/')
  {
    const PersonInfo& p1 = pick_person();
    out = build_divide_share(p1, pick_item(), a, b);
    return true;
  }
  return false;
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
  std::string word_text;
  // ~50% word problems when the numbers make a sensible story; the numeric
  // ranges (and hence grade difficulty) are inherited from above, so
  // multiplication/division stories only appear where x// are allowed.
  const bool want_word = (pick_int(0, 1) == 0);
  if (want_word && try_build_word_problem(op, a, b, word_text))
    qs << word_text;
  else
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
