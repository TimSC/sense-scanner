/********************************************************************************
** Form generated from reading UI file 'shapegui.ui'
**
** Created: Thu Mar 14 10:58:06 2013
**      by: Qt User Interface Compiler version 4.8.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHAPEGUI_H
#define UI_SHAPEGUI_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QDialog>
#include <QtGui/QGroupBox>
#include <QtGui/QHeaderView>
#include <QtGui/QLabel>
#include <QtGui/QPushButton>
#include <QtGui/QVBoxLayout>
#include "shapegui.h"

QT_BEGIN_NAMESPACE

class Ui_ShapeGui
{
public:
    QVBoxLayout *verticalLayout_2;
    QLabel *label;
    QGroupBox *groupBox;
    QVBoxLayout *verticalLayout;
    QListViewWithChanges *shapePresets;
    QPushButton *usePresetButton;
    QPushButton *useSavedShape;
    QPushButton *useCustomShape;

    void setupUi(QDialog *ShapeGui)
    {
        if (ShapeGui->objectName().isEmpty())
            ShapeGui->setObjectName(QString::fromUtf8("ShapeGui"));
        ShapeGui->resize(346, 385);
        verticalLayout_2 = new QVBoxLayout(ShapeGui);
        verticalLayout_2->setObjectName(QString::fromUtf8("verticalLayout_2"));
        label = new QLabel(ShapeGui);
        label->setObjectName(QString::fromUtf8("label"));

        verticalLayout_2->addWidget(label);

        groupBox = new QGroupBox(ShapeGui);
        groupBox->setObjectName(QString::fromUtf8("groupBox"));
        verticalLayout = new QVBoxLayout(groupBox);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        shapePresets = new QListViewWithChanges(groupBox);
        shapePresets->setObjectName(QString::fromUtf8("shapePresets"));

        verticalLayout->addWidget(shapePresets);

        usePresetButton = new QPushButton(groupBox);
        usePresetButton->setObjectName(QString::fromUtf8("usePresetButton"));

        verticalLayout->addWidget(usePresetButton);


        verticalLayout_2->addWidget(groupBox);

        useSavedShape = new QPushButton(ShapeGui);
        useSavedShape->setObjectName(QString::fromUtf8("useSavedShape"));

        verticalLayout_2->addWidget(useSavedShape);

        useCustomShape = new QPushButton(ShapeGui);
        useCustomShape->setObjectName(QString::fromUtf8("useCustomShape"));

        verticalLayout_2->addWidget(useCustomShape);


        retranslateUi(ShapeGui);
        QObject::connect(useCustomShape, SIGNAL(pressed()), ShapeGui, SLOT(UseCustomPressed()));
        QObject::connect(usePresetButton, SIGNAL(pressed()), ShapeGui, SLOT(UsePresetPressed()));
        QObject::connect(useSavedShape, SIGNAL(pressed()), ShapeGui, SLOT(LoadShapePressed()));

        QMetaObject::connectSlotsByName(ShapeGui);
    } // setupUi

    void retranslateUi(QDialog *ShapeGui)
    {
        ShapeGui->setWindowTitle(QApplication::translate("ShapeGui", "No Shape Is Set", 0, QApplication::UnicodeUTF8));
        label->setText(QApplication::translate("ShapeGui", "No tracking shape has been set.", 0, QApplication::UnicodeUTF8));
        groupBox->setTitle(QApplication::translate("ShapeGui", "Use Preset Shape", 0, QApplication::UnicodeUTF8));
        usePresetButton->setText(QApplication::translate("ShapeGui", "Use Preset Shape", 0, QApplication::UnicodeUTF8));
        useSavedShape->setText(QApplication::translate("ShapeGui", "Load Shape from File", 0, QApplication::UnicodeUTF8));
        useCustomShape->setText(QApplication::translate("ShapeGui", "Create Custom Shape", 0, QApplication::UnicodeUTF8));
    } // retranslateUi

};

namespace Ui {
    class ShapeGui: public Ui_ShapeGui {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHAPEGUI_H
