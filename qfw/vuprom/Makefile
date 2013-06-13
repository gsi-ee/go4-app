ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
ifndef GO4SYS
$(error Go4 not found - please call . go4login)
endif
endif


include $(GO4SYS)/Makefile.config

#DOOPTIMIZATION=false

## normally should be like this for every module, but can be specific

QFW_VUPROM_DIR         = .

QFW_VUPROM_LINKDEF     = $(QFW_VUPROM_DIR)/VupromQFWLinkDef.$(HedSuf)
QFW_VUPROM_LIBNAME     = $(GO4_USERLIBNAME)

## must be similar for every module

QFW_VUPROM_DICT        = $(QFW_VUPROM_DIR)/$(DICT_PREFIX)VupromQFW
QFW_VUPROM_DH          = $(QFW_VUPROM_DICT).$(HedSuf)
QFW_VUPROM_DS          = $(QFW_VUPROM_DICT).$(SrcSuf)
QFW_VUPROM_DO          = $(QFW_VUPROM_DICT).$(ObjSuf)

QFW_VUPROM_H           = $(filter-out $(QFW_VUPROM_DH) $(QFW_VUPROM_LINKDEF), $(wildcard $(QFW_VUPROM_DIR)/*.$(HedSuf)))
QFW_VUPROM_S           = $(filter-out $(QFW_VUPROM_DS), $(wildcard $(QFW_VUPROM_DIR)/*.$(SrcSuf)))
QFW_VUPROM_O           = $(QFW_VUPROM_S:.$(SrcSuf)=.$(ObjSuf))

QFW_VUPROM_DEP         =  $(QFW_VUPROM_O:.$(ObjSuf)=.$(DepSuf))
QFW_VUPROM_DDEP        =  $(QFW_VUPROM_DO:.$(ObjSuf)=.$(DepSuf))

QFW_VUPROM_LIB         =  $(QFW_VUPROM_DIR)/$(QFW_VUPROM_LIBNAME).$(DllSuf)

# used in the main Makefile

EXAMPDEPENDENCS    += $(QFW_VUPROM_DEP) $(QFW_VUPROM_DDEP)


##### local rules #####

all::     $(QFW_VUPROM_LIB)

$(QFW_VUPROM_LIB):  $(QFW_VUPROM_O) $(QFW_VUPROM_DO)
	@$(MakeLibrary) $(QFW_VUPROM_LIBNAME) "$(QFW_VUPROM_O) $(QFW_VUPROM_DO)" $(QFW_VUPROM_DIR) $(QFW_VUPROM_LINKDEF) "$(ANAL_LIB_DEP)"

$(QFW_VUPROM_DS): $(QFW_VUPROM_H)  $(QFW_VUPROM_LINKDEF)
	@$(ROOTCINTGO4) $(QFW_VUPROM_H) $(QFW_VUPROM_LINKDEF)

clean-bin::
	@rm -f $(QFW_VUPROM_O) $(QFW_VUPROM_DO)
	@rm -f $(QFW_VUPROM_DEP) $(QFW_VUPROM_DDEP) $(QFW_VUPROM_DS) $(QFW_VUPROM_DH)

clean:: clean-bin
	@$(CleanLib) $(QFW_VUPROM_LIBNAME) $(QFW_VUPROM_DIR)
	@echo "Clean qfw-vuprom project"

include $(GO4SYS)/Makefile.rules
