ifndef GO4SYS
GO4SYS = $(shell go4-config --go4sys)
ifndef GO4SYS
$(error Go4 not found - please call . go4login)
endif
endif


include $(GO4SYS)/Makefile.config

DOOPTIMIZATION=false

## normally should be like this for every module, but can be specific

HitDet_OPTIC_DIR         = .

HitDet_OPTIC_LINKDEF     = $(HitDet_OPTIC_DIR)/HitDetLinkDef.$(HedSuf)
HitDet_OPTIC_LIBNAME     = $(GO4_USERLIBNAME)

## must be similar for every module

HitDet_OPTIC_DICT        = $(HitDet_OPTIC_DIR)/$(DICT_PREFIX)HitDet
HitDet_OPTIC_DH          = $(HitDet_OPTIC_DICT).$(HedSuf)
HitDet_OPTIC_DS          = $(HitDet_OPTIC_DICT).$(SrcSuf)
HitDet_OPTIC_DO          = $(HitDet_OPTIC_DICT).$(ObjSuf)

HitDet_OPTIC_H           = $(filter-out $(HitDet_OPTIC_DH) $(HitDet_OPTIC_LINKDEF), $(wildcard $(HitDet_OPTIC_DIR)/*.$(HedSuf)))
HitDet_OPTIC_S           = $(filter-out $(HitDet_OPTIC_DS), $(wildcard $(HitDet_OPTIC_DIR)/*.$(SrcSuf)))
HitDet_OPTIC_O           = $(HitDet_OPTIC_S:.$(SrcSuf)=.$(ObjSuf))

HitDet_OPTIC_DEP         =  $(HitDet_OPTIC_O:.$(ObjSuf)=.$(DepSuf))
HitDet_OPTIC_DDEP        =  $(HitDet_OPTIC_DO:.$(ObjSuf)=.$(DepSuf))

HitDet_OPTIC_LIB         =  $(HitDet_OPTIC_DIR)/$(HitDet_OPTIC_LIBNAME).$(DllSuf)

# used in the main Makefile

EXAMPDEPENDENCS    += $(HitDet_OPTIC_DEP) $(HitDet_OPTIC_DDEP)


##### local rules #####

all::     $(HitDet_OPTIC_LIB)

$(HitDet_OPTIC_LIB):  $(HitDet_OPTIC_O) $(HitDet_OPTIC_DO)
	@$(MakeLibrary) $(HitDet_OPTIC_LIBNAME) "$(HitDet_OPTIC_O) $(HitDet_OPTIC_DO)" $(HitDet_OPTIC_DIR) $(HitDet_OPTIC_LINKDEF) "$(ANAL_LIB_DEP)"

$(HitDet_OPTIC_DS): $(HitDet_OPTIC_H)  $(HitDet_OPTIC_LINKDEF)
	@$(ROOTCINTGO4) $(HitDet_OPTIC_H) $(HitDet_OPTIC_LINKDEF)

clean-bin::
	@rm -f $(HitDet_OPTIC_O) $(HitDet_OPTIC_DO)
	@rm -f $(HitDet_OPTIC_DEP) $(HitDet_OPTIC_DDEP) $(HitDet_OPTIC_DS) $(HitDet_OPTIC_DH)

clean:: clean-bin
	@$(CleanLib) $(HitDet_OPTIC_LIBNAME) $(HitDet_OPTIC_DIR)
	@echo "Clean HitDetection ASIC monitoring project"

include $(GO4SYS)/Makefile.rules
