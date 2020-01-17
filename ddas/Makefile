ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
endif

include $(GO4SYS)/Makefile.config

DOOPTIMIZATION=false


ifdef GO4PACKAGE
EXUSERSRC_DIR         = Go4ExampleUserSource
else
EXUSERSRC_DIR         = .
endif

NSCL_DIR = $(EXUSERSRC_DIR)/nscl
## use sources taken from nscl git in subfolder:
INCLUDES += -I$(NSCL_DIR)

EXUSERSRC_LINKDEF     = $(EXUSERSRC_DIR)/DDASLinkDef.$(HedSuf)
EXUSERSRC_LIBNAME     = $(GO4_USERLIBNAME)

## must be similar for every module

EXUSERSRC_DICT        = $(EXUSERSRC_DIR)/$(DICT_PREFIX)DDAS
EXUSERSRC_DH          = $(EXUSERSRC_DICT).$(HedSuf)
EXUSERSRC_DS          = $(EXUSERSRC_DICT).$(SrcSuf)
EXUSERSRC_DO          = $(EXUSERSRC_DICT).$(ObjSuf)

EXUSERSRC_H           = $(filter-out $(EXUSERSRC_DH) $(EXUSERSRC_LINKDEF), $(wildcard $(EXUSERSRC_DIR)/*.$(HedSuf)))
EXUSERSRC_S           = $(filter-out $(EXUSERSRC_DS), $(wildcard $(EXUSERSRC_DIR)/*.$(SrcSuf)))
EXUSERSRC_O           = $(EXUSERSRC_S:.$(SrcSuf)=.$(ObjSuf))

EXUSERSRC_DEP         =  $(EXUSERSRC_O:.$(ObjSuf)=.$(DepSuf))
EXUSERSRC_DDEP        =  $(EXUSERSRC_DO:.$(ObjSuf)=.$(DepSuf))

NSCL_H                 = $(wildcard $(NSCL_DIR)/*.$(HedSuf))
NSCL_S                = $(wildcard $(NSCL_DIR)/*.cpp )
NSCL_O                = $(NSCL_S:.cpp=.$(ObjSuf))

NSCL_DEP         =  $(NSCL_O:.$(ObjSuf)=.$(DepSuf))
NSCL_DDEP        =  $(NSCL_DO:.$(ObjSuf)=.$(DepSuf))


EXUSERSRC_LIB         =  $(EXUSERSRC_DIR)/$(EXUSERSRC_LIBNAME).$(DllSuf)

# used in the main Makefile

EXAMPDEPENDENCS    += $(EXUSERSRC_DEP) $(EXUSERSRC_DDEP) $(NSCL_DEP) $(NSCL_DDEP)

ifdef DOPACKAGE
DISTRFILES         += $(EXUSERSRC_S) $(EXUSERSRC_H) $(EXUSERSRC_LINKDEF) $(NSCL_S)  $(NSCL_H)
DISTRFILES         += $(EXUSERSRC_DIR)/README.txt
endif

##### local rules #####

all::  $(EXUSERSRC_LIB)

$(EXUSERSRC_LIB):   $(NSCL_O) $(EXUSERSRC_O) $(EXUSERSRC_DO) $(ANAL_LIB_DEP)
	@$(MakeLibrary) $(EXUSERSRC_LIBNAME) "$(NSCL_O) $(EXUSERSRC_O) $(EXUSERSRC_DO)" $(EXUSERSRC_DIR) $(EXUSERSRC_LINKDEF) "$(ANAL_LIB_DEP)" $(EXUSERSRC_DS) "$(EXUSERSRC_H)"

$(EXUSERSRC_DS): $(EXUSERSRC_H) $(NSCL_H) $(EXUSERSRC_LINKDEF)
	@$(ROOTCINTGO4) $(EXUSERSRC_LIB) $(EXUSERSRC_H) $(NSCL_H) $(EXUSERSRC_LINKDEF)

clean-bin::
	@$(RM) $(EXUSERSRC_O) $(EXUSERSRC_DO) $(NSCL_O)
	@$(RM) $(EXUSERSRC_DEP) $(EXUSERSRC_DDEP) $(NSCL_DEP) $(NSCL_DDEP)$(EXUSERSRC_DS) $(EXUSERSRC_DH)

clean:: clean-bin
	@$(CleanLib) $(EXUSERSRC_LIBNAME) $(EXUSERSRC_DIR)

include $(GO4SYS)/Makefile.rules
