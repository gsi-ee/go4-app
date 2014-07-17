ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
ifndef GO4SYS
$(error Go4 not found - please call . go4login)
endif
endif


include $(GO4SYS)/Makefile.config

DOOPTIMIZATION=false

## normally should be like this for every module, but can be specific

QFW_OPTIC_DIR         = .

QFW_OPTIC_LINKDEF     = $(QFW_OPTIC_DIR)/QFWLinkDef.$(HedSuf)
QFW_OPTIC_LIBNAME     = $(GO4_USERLIBNAME)

## must be similar for every module

QFW_OPTIC_DICT        = $(QFW_OPTIC_DIR)/$(DICT_PREFIX)QFW
QFW_OPTIC_DH          = $(QFW_OPTIC_DICT).$(HedSuf)
QFW_OPTIC_DS          = $(QFW_OPTIC_DICT).$(SrcSuf)
QFW_OPTIC_DO          = $(QFW_OPTIC_DICT).$(ObjSuf)

QFW_OPTIC_H           = $(filter-out $(QFW_OPTIC_DH) $(QFW_OPTIC_LINKDEF), $(wildcard $(QFW_OPTIC_DIR)/*.$(HedSuf)))
QFW_OPTIC_S           = $(filter-out $(QFW_OPTIC_DS), $(wildcard $(QFW_OPTIC_DIR)/*.$(SrcSuf)))
QFW_OPTIC_O           = $(QFW_OPTIC_S:.$(SrcSuf)=.$(ObjSuf))

QFW_OPTIC_DEP         =  $(QFW_OPTIC_O:.$(ObjSuf)=.$(DepSuf))
QFW_OPTIC_DDEP        =  $(QFW_OPTIC_DO:.$(ObjSuf)=.$(DepSuf))

QFW_OPTIC_LIB         =  $(QFW_OPTIC_DIR)/$(QFW_OPTIC_LIBNAME).$(DllSuf)

# used in the main Makefile

EXAMPDEPENDENCS    += $(QFW_OPTIC_DEP) $(QFW_OPTIC_DDEP)


##### local rules #####

all::     $(QFW_OPTIC_LIB)

$(QFW_OPTIC_LIB):  $(QFW_OPTIC_O) $(QFW_OPTIC_DO)
	@$(MakeLibrary) $(QFW_OPTIC_LIBNAME) "$(QFW_OPTIC_O) $(QFW_OPTIC_DO)" $(QFW_OPTIC_DIR) $(QFW_OPTIC_LINKDEF) "$(ANAL_LIB_DEP)"

$(QFW_OPTIC_DS): $(QFW_OPTIC_H)  $(QFW_OPTIC_LINKDEF)
	@$(ROOTCINTGO4) $(QFW_OPTIC_H) $(QFW_OPTIC_LINKDEF)

clean-bin::
	@rm -f $(QFW_OPTIC_O) $(QFW_OPTIC_DO)
	@rm -f $(QFW_OPTIC_DEP) $(QFW_OPTIC_DDEP) $(QFW_OPTIC_DS) $(QFW_OPTIC_DH)

clean:: clean-bin
	@$(CleanLib) $(QFW_OPTIC_LIBNAME) $(QFW_OPTIC_DIR)
	@echo "Clean qfw-profilemonitor project"

include $(GO4SYS)/Makefile.rules
