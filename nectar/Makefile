ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
ifndef GO4SYS
$(error Go4 not found - please call . go4login)
endif
endif


include $(GO4SYS)/Makefile.config

DOOPTIMIZATION=false

## normally should be like this for every module, but can be specific

Nectar_OPTIC_DIR         = .

Nectar_OPTIC_LINKDEF     = $(Nectar_OPTIC_DIR)/NectarLinkDef.$(HedSuf)
Nectar_OPTIC_LIBNAME     = $(GO4_USERLIBNAME)

## must be similar for every module

Nectar_OPTIC_DICT        = $(Nectar_OPTIC_DIR)/$(DICT_PREFIX)Nectar
Nectar_OPTIC_DH          = $(Nectar_OPTIC_DICT).$(HedSuf)
Nectar_OPTIC_DS          = $(Nectar_OPTIC_DICT).$(SrcSuf)
Nectar_OPTIC_DO          = $(Nectar_OPTIC_DICT).$(ObjSuf)

Nectar_OPTIC_H           = $(filter-out $(Nectar_OPTIC_DH) $(Nectar_OPTIC_LINKDEF), $(wildcard $(Nectar_OPTIC_DIR)/*.$(HedSuf)))
Nectar_OPTIC_S           = $(filter-out $(Nectar_OPTIC_DS), $(wildcard $(Nectar_OPTIC_DIR)/*.$(SrcSuf)))
Nectar_OPTIC_O           = $(Nectar_OPTIC_S:.$(SrcSuf)=.$(ObjSuf))

Nectar_OPTIC_DEP         =  $(Nectar_OPTIC_O:.$(ObjSuf)=.$(DepSuf))
Nectar_OPTIC_DDEP        =  $(Nectar_OPTIC_DO:.$(ObjSuf)=.$(DepSuf))

Nectar_OPTIC_LIB         =  $(Nectar_OPTIC_DIR)/$(Nectar_OPTIC_LIBNAME).$(DllSuf)

# used in the main Makefile

EXAMPDEPENDENCS    += $(Nectar_OPTIC_DEP) $(Nectar_OPTIC_DDEP)


##### local rules #####

all::     $(Nectar_OPTIC_LIB)

$(Nectar_OPTIC_LIB):  $(Nectar_OPTIC_O) $(Nectar_OPTIC_DO) $(ANAL_LIB_DEP)
	@$(MakeLibrary) $(Nectar_OPTIC_LIBNAME) "$(Nectar_OPTIC_O) $(Nectar_OPTIC_DO)" $(Nectar_OPTIC_DIR) $(Nectar_OPTIC_LINKDEF) "$(ANAL_LIB_DEP)" $(Nectar_OPTIC_DS) "$(Nectar_OPTIC_H)"



$(Nectar_OPTIC_DS): $(Nectar_OPTIC_H)  $(Nectar_OPTIC_LINKDEF)
	@$(ROOTCINTGO4) $(Nectar_OPTIC_LIB) $(Nectar_OPTIC_H) $(Nectar_OPTIC_LINKDEF)

clean-bin::
	@rm -f $(Nectar_OPTIC_O) $(Nectar_OPTIC_DO)
	@rm -f $(Nectar_OPTIC_DEP) $(Nectar_OPTIC_DDEP) $(Nectar_OPTIC_DS) $(Nectar_OPTIC_DH)

clean:: clean-bin
	@$(CleanLib) $(Nectar_OPTIC_LIBNAME) $(Nectar_OPTIC_DIR)
	@echo "Clean Nectarection ASIC monitoring project"

include $(GO4SYS)/Makefile.rules
