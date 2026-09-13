//  SuperTux - SuperMathTux feature
//  Modal multiple-choice math quiz menu shown during gameplay.
#pragma once

#include <functional>

#include "gui/menu.hpp"
#include "supertux/math_question.hpp"

class MathQuizMenu final : public Menu
{
public:
  /** on_answered(true) = correct, on_answered(false) = wrong. Always called exactly once. */
  MathQuizMenu(const MathQuestion& question,
               std::function<void(bool)> on_answered);
  ~MathQuizMenu() override = default;

  void menu_action(MenuItem& item) override;
  bool on_back_action() override { return false; } // Force an answer; no ESC skip.

private:
  MathQuestion m_question;
  std::function<void(bool)> m_on_answered;
  bool m_answered = false;

  void answer(int index);
};
