/* This file is part of COVISE.

You can use it under the terms of the GNU Lesser General Public License
version 2.1 or later, see lgpl-2.1.txt.

* License: LGPL 2+ */
#ifndef OSC_CONDITION_CHOICE_OBJECT_H
#define OSC_CONDITION_CHOICE_OBJECT_H
#include <oscExport.h>
#include <oscObjectBase.h>
#include <oscObjectVariable.h>
#include <oscVariables.h>

namespace OpenScenario {

class OPENSCENARIOEXPORT conditionType: public oscEnumType
{
public:
    static conditionType *instance(); 
private:
    conditionType();
    static conditionType *inst;
};

/// \class This class represents a generic OpenScenario Object
class OPENSCENARIOEXPORT oscConditionChoiceObject: public oscObjectBase
{
public:
	
    enum condition
    {
        exceed,
		deceed,
    };
    oscConditionChoiceObject()
    {
		OSC_ADD_MEMBER(object);
		OSC_ADD_MEMBER(refObject);
		OSC_ADD_MEMBER(value);
		OSC_ADD_MEMBER(condition);
		condition.enumType = conditionType::instance();
    };
	oscString object;
	oscString refObject;
	oscDouble value;
	oscEnum condition;
};

typedef oscObjectVariable<oscConditionChoiceObject *> oscConditionChoiceObjectMember;

}

#endif //OSC_CONDITION_CHOICE_OBJECT_H
