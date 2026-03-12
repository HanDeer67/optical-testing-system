#include "numberlineedit.h"
#include <QDoubleValidator>

NumberLineEdit::NumberLineEdit(QWidget *parent): QLineEdit(parent)
{
    setup();
}

void NumberLineEdit::setup(int maxLength, int decimals){
    QDoubleValidator *validator = new QDoubleValidator(this);
    validator->setDecimals(decimals);
    validator->setNotation(QDoubleValidator::StandardNotation);

    setValidator(validator);
    setMaxLength(maxLength);
}
