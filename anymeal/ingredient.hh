#pragma once
#include <string>


class Ingredient
{
public:
  Ingredient(void);
  int amount_integer(void) { return m_amount_integer; }
  void set_amount_integer(int amount_integer) { m_amount_integer = amount_integer; }
  int amount_numerator(void) { return m_amount_numerator; }
  void set_amount_numerator(int amount_numerator) { m_amount_numerator = amount_numerator; }
  int amount_denominator(void) { return m_amount_denominator; }
  void set_amount_denominator(int amount_denominator) { m_amount_denominator = amount_denominator; }
  double amount_float(void) { return m_amount_float; }
  void set_amount_float(double amount_float) { m_amount_float = amount_float; }
  std::string &unit(void) { return m_unit; }
  void set_unit(const char *unit) { m_unit = unit; }
  std::string &text(void) { return m_text; }
  void add_text(const char *text) { m_text += text; }
protected:
  int m_amount_integer;
  int m_amount_numerator;
  int m_amount_denominator;
  double m_amount_float;
  std::string m_unit;
  std::string m_text;
};