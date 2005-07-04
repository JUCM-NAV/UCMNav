# Makefile without dependencies

# Modify the makeflags file to set appropriate directories
include makeflags

GRAPHICAL_FILES = main.cc bspline.cc handle.cc callbacks.cc ucmnavui.c presentation.cc xfpresentation.cc \
	pspresentation.cc cgmpresentation.cc point.cc dynarrow.cc FL_Responsibility.cc FL_Printer.cc FL_CompAttr.cc \
	FL_MainWindow.cc  FL_StructDyn.cc FL_Conditions.cc FL_Stub.cc FL_Timestamp.cc FL_Device.cc FL_DataStores.cc \
	FL_DataUsage.cc FL_ArrivalProcess.cc FL_Poolplugin.cc FL_Goal.cc FL_ServiceRequests.cc FL_SelectMap.cc FL_ParentMapList.cc \
	FL_UcmSet.cc FL_ReferenceReplacement.cc FL_Utilities.cc FL_ForkSpec.cc FL_BooleanVariables.cc FL_Scenario.cc FL_ConditionEditor.cc \
	FL_VariableOperations.cc figure.cc hyperedge_figure.cc point_figure.cc marker_figure.cc or_null_figure.cc synch_null_figure.cc \
        loop_null_figure.cc path.cc resp_figure.cc stub_figure.cc or_figure.cc synch_figure.cc map.cc ts_figure.cc goal_figure.cc loop_figure.cc \
	mifpresentation.cc svgpresentation.cc print_mgr.cc

GRAPHICAL_OBJFILES = main.o bspline.o handle.o callbacks.o presentation.o xfpresentation.o \
	pspresentation.o cgmpresentation.o point.o dynarrow.o FL_Responsibility.o FL_Printer.o FL_CompAttr.o \
	FL_StructDyn.o FL_MainWindow.o FL_Conditions.o FL_Stub.o FL_Timestamp.o FL_Device.o FL_DataStores.o \
	FL_DataUsage.o FL_ArrivalProcess.o FL_Poolplugin.o FL_Goal.o FL_ServiceRequests.o FL_SelectMap.o FL_ParentMapList.o \
	FL_UcmSet.o FL_ReferenceReplacement.o FL_Utilities.o FL_ForkSpec.o FL_BooleanVariables.o FL_Scenario.o FL_ConditionEditor.o \
	FL_VariableOperations.o  map.o figure.o hyperedge_figure.o point_figure.o or_null_figure.o synch_null_figure.o loop_null_figure.o  \
        path.o resp_figure.o stub_figure.o or_figure.o synch_figure.o marker_figure.o ts_figure.o goal_figure.o loop_figure.o ucmnavui.o \
	mifpresentation.o print_mgr.o svgpresentation.o 

HYPEREDGE_FILES = connect.cc display.cc empty.cc hyperedge.cc hypergraph.cc  component.cc component_ref.cc label.cc \
		  loop.cc node.cc or_join.cc resp_ref.cc result.cc start.cc stub.cc timer.cc transformation.cc wait.cc waiting_place.cc \
		  synchronization.cc or_fork.cc goal_tag.cc condition.cc timestamp.cc response_time.cc devices.cc data.cc goal.cc \
		  responsibility.cc resp_mgr.cc component_mgr.cc ucm_set.cc msc_generator.cc service.cc variable.cc scenario.cc wait_synch.cc \
		  action.cc path_data.cc scenario_generator.cc conditions.cc
		  
HYPEREDGE_OBJFILES = connect.o display.o empty.o hyperedge.o hypergraph.o component.o component_ref.o label.o loop.o node.o \
		     synchronization.o or_fork.o or_join.o resp_ref.o result.o start.o stub.o timer.o transformation.o wait.o waiting_place.o \
		     goal_tag.o condition.o timestamp.o response_time.o devices.o data.o goal.o responsibility.o resp_mgr.o \
		     component_mgr.o ucm_set.o msc_generator.o service.o variable.o scenario.o wait_synch.o lex.evl.o evaluate.tab.o \
		     action.o path_data.o scenario_generator.o conditions.o
#		     lex.pfm.o performance.tab.o 

XML_CFILES = xml_mgr.cc # xmltok.c xmlparse.c hashtable.c xmlrole.c

XML_OBJFILES = xml_mgr.o # xmltok.o xmlparse.o hashtable.o xmlrole.o

$(UCMNAV_ADD_ON)_CFILES = lqngenerator.cc lqn.cc lqnactivity.cc lqndevice.cc lqnentry.cc lqntask.cc lqnmessage.cc lqnmstack.cc
$(UCMNAV_ADD_ON)_OBJFILES = lqngenerator.o lqn.o lqnactivity.o lqndevice.o lqnentry.o lqntask.o lqnmessage.o lqnmstack.o

CFILES = $(HYPEREDGE_FILES) $(GRAPHICAL_FILES) $(XML_CFILES) $($(UCMNAV_ADD_ON)_CFILES)
OBJFILES = $(HYPEREDGE_OBJFILES) $(GRAPHICAL_OBJFILES) $(XML_OBJFILES) $($(UCMNAV_ADD_ON)_OBJFILES)

SINGLE = callbacks.cc

all: ucmnav $(OBJFILES) $(CFILES) 

compile: $(OBJFILES)
	# Program files simply compiled.

single: $(SINGLE)
	$(CPP) -E $(CFLAGS) $< > preprocessed

ucmnav: $(OBJFILES)
	$(CPP) $(CFLAGS) -o $(BINDIR)/ucmnav $(OBJFILES) $(LIBS) 2>&1

version:
	$(CPP) -V

.SUFFIXES: .cc .c

.cc.o:
	$(CPP) $(CPPFLAGS) -DUCMNAV_HOME=$(UCMNAV_HOME) -c  2>&1 $<

.c.o:
	$(CC) $(CFLAGS) -DUCMNAV_HOME=$(UCMNAV_HOME) -c  2>&1 $<

ucmnavui.c: ucmnavui.fd
	fdesign -convert ucmnavui.fd

ucmnavui.h: ucmnavui.c

display.cc: ucmnavui.h

evaluate.tab.c: evaluate.y
	bison -y -d -p evl evaluate.y
	cp y.tab.h evaluate.tab.h
	cp y.tab.c evaluate.tab.c

evaluate.tab.h: evaluate.tab.c 

evaluate.tab.o: evaluate.tab.c
	$(CC) $(CFLAGS) -Dyywrap=evlwrap -c  2>&1 $<

lex.evl.c: evaluate.l
	flex -Pevl evaluate.l

#lex.pfm.c: performance.l
#	flex -Ppfm performance.l

#performance.tab.c: performance.y
#	bison -d -p pfm performance.y

clean:
	$(CLEAN_OPERATIONS) ;\
	make all

tar:
	tar -cvf ucmnav-code.tar *.cc *.c *.h *.fd *.txt Makefile makeflags ps-header.ps   ; \
	gzip ucmnav-code.tar ; \
	mv ucmnav-code.tar.gz $HOME/backup

lex.evl.o: lex.evl.c evaluate.tab.h
#lex.pfm.o: lex.pfm.c performance.tab.h
bspline.o: bspline.h
interpolate.o: interpolate.cc
callbacks.o: callbacks.cc
ucmnavui.o: ucmnavui.c
FL_ArrivalProcess.o: FL_ArrivalProcess.cc
FL_DataStores.o: FL_DataStores.cc
FL_MainWindow.o: FL_MainWindow.cc
FL_StructDyn.o: FL_StructDyn.cc
FL_CompAttr.o: FL_CompAttr.cc
FL_DataUsage.o: FL_DataUsage.cc
FL_Printer.o: FL_Printer.cc
FL_Stub.o: FL_Stub.cc
FL_Conditions.o: FL_Conditions.cc
FL_Device.o: FL_Device.cc
FL_Responsibility.o: FL_Responsibility.cc
FL_Timestamp.o: FL_Timestamp.cc
handle.o: handle.h
component.o: component.h
presentation.o: presentation.h
xfpresentation.o: xfpresentation.h
pspresentation.o: pspresentation.h
point.o: point.h
dynarrow.o: dynarrow.h

action.o: action.h
path_data.o: path_data.h
scenario_generator.o: scenario_generator.h
map.o: map.h
figure.o: figure.h
point_figure.o: point_figure.h
marker_figure.o: marker_figure.h
or_null_figure.o: or_null_figure.h
synch_null_figure.o: synch_null_figure.h
component_mgr.o:  component_mgr.h
path.o: path.h
resp_figure.o: resp_figure.h
stub_figure.o: stub_figure.h
or_figure.o: or_figure.h
synch_figure.o: synch_figure.h
file_mgr.o: file_mgr.h
ts_figure.o: ts_figure.h
connect.o: connect.h
display.o: display.h
empty.o: empty.h
hyperedge.o: hyperedge.h
hyperedge_figure.o: hyperedge_figure.h
hypergraph.o: hypergraph.h
label.o: label.h
labeller.o: labeller.h
node.o: node.h
or_fork.o: or_fork.h
or_join.o: or_join.h
responsibility.o: responsibility.h
result.o: result.h
start.o: start.h
stub.o: stub.h
wait_synch.o: wait_synch.h
timer.o: timer.h
transformation.o: transformation.h
wait.o: wait.h
waiting_place.o: waiting_place.h
synchronization.o: synchronization.h 
condition.o: condition.h
timestamp.o: timestamp.h
response_time.o: response_time.h
devices.o: devices.h
data.o: data.h
main.o: templates-def.cc
xml_mgr.o: xml_mgr.h
goal_tag.o: goal_tag.h
goal.o: goal.h
mifpresentation.o: mifpresentation.h
loop.o: loop.h
loop_figure.o: loop_figure.h
variable.o: variable.h
# for LQN generation
lqngenerator.o: lqngenerator.h
lqn.o: lqn.h
lqnactivity.o: lqnactivity.h
lqndevice.o: lqndevice.h
lqnentry.o: lqnentry.h
lqnmessage.o: lqnmessage.h
lqnmstack.o: lqnmstack.h
lqntask.o: lqntask.h
