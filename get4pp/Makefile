ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
ifndef GO4SYS
$(error Go4 not found - please call . go4login)
endif
endif


include $(GO4SYS)/Makefile.config

DOOPTIMIZATION=false

## normally should be like this for every module, but can be specific

Get4pp_OPTIC_DIR         = .

Get4pp_OPTIC_LINKDEF     = $(Get4pp_OPTIC_DIR)/Get4ppLinkDef.$(HedSuf)
Get4pp_OPTIC_LIBNAME     = $(GO4_USERLIBNAME)

## must be similar for every module

Get4pp_OPTIC_DICT        = $(Get4pp_OPTIC_DIR)/$(DICT_PREFIX)Get4pp
Get4pp_OPTIC_DH          = $(Get4pp_OPTIC_DICT).$(HedSuf)
Get4pp_OPTIC_DS          = $(Get4pp_OPTIC_DICT).$(SrcSuf)
Get4pp_OPTIC_DO          = $(Get4pp_OPTIC_DICT).$(ObjSuf)

Get4pp_OPTIC_H           = $(filter-out $(Get4pp_OPTIC_DH) $(Get4pp_OPTIC_LINKDEF), $(wildcard $(Get4pp_OPTIC_DIR)/*.$(HedSuf)))
Get4pp_OPTIC_S           = $(filter-out $(Get4pp_OPTIC_DS), $(wildcard $(Get4pp_OPTIC_DIR)/*.$(SrcSuf)))
Get4pp_OPTIC_O           = $(Get4pp_OPTIC_S:.$(SrcSuf)=.$(ObjSuf))

Get4pp_OPTIC_DEP         =  $(Get4pp_OPTIC_O:.$(ObjSuf)=.$(DepSuf))
Get4pp_OPTIC_DDEP        =  $(Get4pp_OPTIC_DO:.$(ObjSuf)=.$(DepSuf))

Get4pp_OPTIC_LIB         =  $(Get4pp_OPTIC_DIR)/$(Get4pp_OPTIC_LIBNAME).$(DllSuf)

# used in the main Makefile

EXAMPDEPENDENCS    += $(Get4pp_OPTIC_DEP) $(Get4pp_OPTIC_DDEP)


##### local rules #####

all::     $(Get4pp_OPTIC_LIB)

$(Get4pp_OPTIC_LIB):  $(Get4pp_OPTIC_O) $(Get4pp_OPTIC_DO) $(ANAL_LIB_DEP)
	@$(MakeLibrary) $(Get4pp_OPTIC_LIBNAME) "$(Get4pp_OPTIC_O) $(Get4pp_OPTIC_DO)" $(Get4pp_OPTIC_DIR) $(Get4pp_OPTIC_LINKDEF) "$(ANAL_LIB_DEP)" $(Get4pp_OPTIC_DS) "$(Get4pp_OPTIC_H)"



$(Get4pp_OPTIC_DS): $(Get4pp_OPTIC_H)  $(Get4pp_OPTIC_LINKDEF)
	@$(ROOTCINTGO4) $(Get4pp_OPTIC_LIB) $(Get4pp_OPTIC_H) $(Get4pp_OPTIC_LINKDEF)

clean-bin::
	@rm -f $(Get4pp_OPTIC_O) $(Get4pp_OPTIC_DO)
	@rm -f $(Get4pp_OPTIC_DEP) $(Get4pp_OPTIC_DDEP) $(Get4pp_OPTIC_DS) $(Get4pp_OPTIC_DH)

clean:: clean-bin
	@$(CleanLib) $(Get4pp_OPTIC_LIBNAME) $(Get4pp_OPTIC_DIR)
	@echo "Clean Get4pp ASIC monitoring project"

include $(GO4SYS)/Makefile.rules
