ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
ifndef GO4SYS
$(error Go4 not found - please call . go4login)
endif
endif


include $(GO4SYS)/Makefile.config

DOOPTIMIZATION=false

## normally should be like this for every module, but can be specific

Ctr16_OPTIC_DIR         = .

Ctr16_OPTIC_LINKDEF     = $(Ctr16_OPTIC_DIR)/Ctr16LinkDef.$(HedSuf)
Ctr16_OPTIC_LIBNAME     = $(GO4_USERLIBNAME)

## must be similar for every module

Ctr16_OPTIC_DICT        = $(Ctr16_OPTIC_DIR)/$(DICT_PREFIX)Ctr16
Ctr16_OPTIC_DH          = $(Ctr16_OPTIC_DICT).$(HedSuf)
Ctr16_OPTIC_DS          = $(Ctr16_OPTIC_DICT).$(SrcSuf)
Ctr16_OPTIC_DO          = $(Ctr16_OPTIC_DICT).$(ObjSuf)

Ctr16_OPTIC_H           = $(filter-out $(Ctr16_OPTIC_DH) $(Ctr16_OPTIC_LINKDEF), $(wildcard $(Ctr16_OPTIC_DIR)/*.$(HedSuf)))
Ctr16_OPTIC_S           = $(filter-out $(Ctr16_OPTIC_DS), $(wildcard $(Ctr16_OPTIC_DIR)/*.$(SrcSuf)))
Ctr16_OPTIC_O           = $(Ctr16_OPTIC_S:.$(SrcSuf)=.$(ObjSuf))

Ctr16_OPTIC_DEP         =  $(Ctr16_OPTIC_O:.$(ObjSuf)=.$(DepSuf))
Ctr16_OPTIC_DDEP        =  $(Ctr16_OPTIC_DO:.$(ObjSuf)=.$(DepSuf))

Ctr16_OPTIC_LIB         =  $(Ctr16_OPTIC_DIR)/$(Ctr16_OPTIC_LIBNAME).$(DllSuf)

# used in the main Makefile

EXAMPDEPENDENCS    += $(Ctr16_OPTIC_DEP) $(Ctr16_OPTIC_DDEP)


##### local rules #####

all::     $(Ctr16_OPTIC_LIB)

$(Ctr16_OPTIC_LIB):  $(Ctr16_OPTIC_O) $(Ctr16_OPTIC_DO) $(ANAL_LIB_DEP)
	@$(MakeLibrary) $(Ctr16_OPTIC_LIBNAME) "$(Ctr16_OPTIC_O) $(Ctr16_OPTIC_DO)" $(Ctr16_OPTIC_DIR) $(Ctr16_OPTIC_LINKDEF) "$(ANAL_LIB_DEP)" $(Ctr16_OPTIC_DS) "$(Ctr16_OPTIC_H)"



$(Ctr16_OPTIC_DS): $(Ctr16_OPTIC_H)  $(Ctr16_OPTIC_LINKDEF)
	@$(ROOTCINTGO4) $(Ctr16_OPTIC_LIB) $(Ctr16_OPTIC_H) $(Ctr16_OPTIC_LINKDEF)

clean-bin::
	@rm -f $(Ctr16_OPTIC_O) $(Ctr16_OPTIC_DO)
	@rm -f $(Ctr16_OPTIC_DEP) $(Ctr16_OPTIC_DDEP) $(Ctr16_OPTIC_DS) $(Ctr16_OPTIC_DH)

clean:: clean-bin
	@$(CleanLib) $(Ctr16_OPTIC_LIBNAME) $(Ctr16_OPTIC_DIR)
	@echo "Clean Ctr16 ASIC monitoring project"

include $(GO4SYS)/Makefile.rules
