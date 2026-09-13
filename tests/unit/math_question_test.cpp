// SuperMathTux: basic sanity test for MathQuestion::generate().
#include <cassert>
#include <iostream>

#include "supertux/math_question.hpp"

int main()
{
  for (int grade = 1; grade <= 6; ++grade)
  {
    for (int i = 0; i < 50; ++i)
    {
      MathQuestion q = MathQuestion::generate(grade);
      assert(q.answers.size() == 4);
      assert(q.correct_index >= 0 && q.correct_index < 4);
      assert(q.answers[static_cast<size_t>(q.correct_index)] == std::to_string(q.correct_value));
      // Answers must be unique.
      for (size_t a = 0; a < q.answers.size(); ++a)
        for (size_t b = a + 1; b < q.answers.size(); ++b)
          assert(q.answers[a] != q.answers[b]);
      assert(!q.question_text.empty());
      // Grades 1-4 must not contain negative answers.
      if (grade <= 4)
      {
        assert(q.correct_value >= 0);
        for (const auto& ans : q.answers)
          assert(!ans.empty() && ans[0] != '-');
      }
    }
  }
  std::cout << "math_question_test passed" << std::endl;
  return 0;
}
