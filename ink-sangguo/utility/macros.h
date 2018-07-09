#pragma once

#define DEFINE_PROPERTY(varType, varName, funName)\
protected: varType varName;\
public: varType Get##funName(void) const { return varName; }\
public: void Set##funName(varType var){ varName = var; }

#define DEFINE_PROPERTY_REF(varType, varName, funName)\
protected: varType varName;\
public: varType& Get##funName(void) { return varName; }\
public: void Set##funName(const varType& var){ varName = var; }

#define DEFINE_STATIC_UID(varType, varName)\
protected: static varType varName;\
public: static varType GenerateUID(void) { varName++; return varName;}\
public: static void SetUID(const varType& var){ varName = var; }

#define DEFINE_STATIC_PRO_REF(varType, varName, funName)\
protected: static varType varName;\
public: static varType& Get##funName(void) { return varName; }\
public: static void Set##funName(const varType& var){ varName = var; }

#define sanguo_max(a,b) (((a) > (b)) ? (a) : (b))
#define sanguo_min(a,b) (((a) < (b)) ? (a) : (b))