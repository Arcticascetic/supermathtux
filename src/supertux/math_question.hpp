//  SuperTux - SuperMathTux feature
//  Math question generator with programmable difficulty (grade levels).
#pragma once

#include <string>
#include <vector>

struct MathQuestion
{
  std::string question_text;
  std::vector<std::string> answers;
  int correct_index = 0;
  int correct_value = 0;

  /** Generate a multiple-choice question for the given grade level (1+).
      Grade 1: addition/subtraction, small numbers (0-10)
      Grade 2: addition/subtraction, larger numbers (0-100)
      Grade 3: addition/subtraction/multiplication
      Grade 4+: addition/subtraction/multiplication/division (harder ranges scale up)
      About half of the plain arithmetic questions are rendered as word
      problems reusing the same grade-appropriate numbers (give-away /
      receive-gift / missing-give for +/-; gifts-from-friends for x;
      share-equally for /).
      The other puzzle types reuse the same grade-appropriate numbers:
      missing-number (? + b = c), number sequences (2, 4, 6, ?),
      bigger/smaller than X, and even/odd choice. */
  static MathQuestion generate(int grade_level);
};
