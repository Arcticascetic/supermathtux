//  SuperTux - SuperMathTux feature
#include "supertux/menu/math_quiz_menu.hpp"

#include "gui/menu_manager.hpp"
#include "util/gettext.hpp"

MathQuizMenu::MathQuizMenu(const MathQuestion& question,
                           std::function<void(bool)> on_answered) :
  m_question(question),
  m_on_answered(std::move(on_answered))
{
  add_label(_("Math Challenge!"));
  add_hl();
  add_inactive(m_question.question_text, true);
  add_hl();
  for (size_t i = 0; i < m_question.answers.size(); ++i)
  {
    const int idx = static_cast<int>(i);
    add_entry(m_question.answers[i], [this, idx]() { answer(idx); });
  }
  add_hl();
  add_inactive(_("Choose the correct answer!"));
  on_window_resize();
}

void
MathQuizMenu::menu_action(MenuItem&)
{
  // Answers are handled via per-entry callbacks.
}

void
MathQuizMenu::answer(int index)
{
  if (m_answered)
    return;
  m_answered = true;
  const bool correct = (index == m_question.correct_index);
  // Copy callback before popping (this may be destroyed).
  auto cb = m_on_answered;
  MenuManager::instance().pop_menu(true);
  if (cb)
    cb(correct);
}
