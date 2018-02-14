
EXECPREFIX=

CC=$(EXECPREFIX) C:/opt/gcc-4.9-win64/bin/gcc.exe
DERIVED_CFLAGS=
CONF_CFLAGS=-I../fmi -I /include -I C:/INTO-CPS/INTO_CPS_Pilot_Studies/case-study_ether/Test-Data-Generation/Ether_Indirect_Test_Model_v0.41/inc -I C:/INTO-CPS/INTO_CPS_Pilot_Studies/case-study_ether/Test-Data-Generation/Ether_Indirect_Test_Model_v0.41/sut -URTT_DEBUG_ENGINE_INTERNAL -URTT_DEBUG_ENGINE_CALLS -URTT_DEBUG_ENGINE_SHM -U_DEBUG_LOGGING -DRTT_FMI_INCLUDE_GET_MAX_STEPSIZE -DRTT_FMI_OUTPUT_TAGS_AT_END -UDEBUG_RTT_FMI_TAGS2STDERR -URTT_FMI_TAGS2RTTINTERFACE_LOG_TAGS -D__MINGW32__ -DRTT_LINGW -g -Wall -I../inc -I../../inc -I C:/INTO-CPS/INTO_CPS_Pilot_Studies/case-study_ether/Test-Data-Generation/Ether_Indirect_Test_Model_v0.41/fmi -DAM_TIMETICK_PREFIX=fmi_timetick

RTTLIBDIR=C:\opt\rt-tester\family\MINGW\lib64
UNIT_OBJ_FILES= declarations.rts.o operations.rts.o rttSignalChecker.rts.o stimulator.rts.o test.rts.o _ora_End1I.rts.o _ora_End2I.rts.o _ora_EtherI.rts.o fmi2adapter.rts.o fmi2rttInterface.rts.o  RTTcontext.o RTTstartup.o RTTschedule.o
CONF_OBJ_FILES= 
CONF_LDPATH=
CONF_LDFLAGS=

OBJECTS=$(UNIT_OBJ_FILES) $(CONF_OBJ_FILES)

CFLAGS=-I.        -Wall        -I$(RTTDIR)/include        $(DERIVED_CFLAGS)        $(CONF_CFLAGS)        -DRTT_POSIX_STD_BUILD         -DRTT_POSIX_STD_BUILD

INCPATH=

OPT_SIGLIBS=-lrttsiglib -lrttvlclib

FAMILY_LIBS=-lwsock32

## -------------------------------------------------------------------------

.PHONY: all 

all: rtt-test-case

.PHONY: ojb

obj: $(OBJECTS)

rtt-test-case: Makefile  $(OBJECTS)
	$(CC) -o rtt-test-case 	$(CFLAGS) 	$(OBJECTS) 	$(CONF_LDPATH) 	$(CONF_LDFLAGS) 	-L$(RTTLIBDIR) 	-lrttctxlib -lrttstrlib  -lrttlogreplay 	-lrttisetlib -lrttservicelib            $(OPT_SIGLIBS)           $(FAMILY_LIBS) 	-Wl,-Map,mapfile.txt


clean:
	@rm -f *.o 		rtt-test-case 		watchdog.* 	        *.bak.[0-9]* 		*.da *.bb *.bb *.bbg 		*.dep *.d 		mapfile.txt

$(UNIT_OBJ_FILES): %.o: %.c Makefile 
	$(CC) $(CFLAGS) $(INCPATH) -MMD -c $< -o $@
	@echo "... ok."

## ----------------------------------------------------

.PHONY: show

show: 	rtt-test-case
	nm --print-armap $<

## ----------------------------------------------------

