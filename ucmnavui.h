/** Header file generated with fdesign on Tue Oct 14 12:17:57 2003.**/

#ifndef FD_Main_h_
#define FD_Main_h_

/** Callbacks, globals and object handlers **/
extern void Responsibilities_cb(FL_OBJECT *, long);
extern void Title_cb(FL_OBJECT *, long);
extern void Description_cb(FL_OBJECT *, long);
extern void ComponentButton_cb(FL_OBJECT *, long);
extern void PathButton_cb(FL_OBJECT *, long);
extern void SelectButton_cb(FL_OBJECT *, long);
extern void DecompositionLevel_cb(FL_OBJECT *, long);
extern void Conditions_Delete_cb(FL_OBJECT *, long);
extern void Conditions_Edit_cb(FL_OBJECT *, long);
extern void EditResponsibility_cb(FL_OBJECT *, long);
extern void Conditions_cb(FL_OBJECT *, long);
extern void Conditions_Add_cb(FL_OBJECT *, long);
extern void AboutMenu_cb(FL_OBJECT *, long);
extern void HorizontalScrollbar_cb(FL_OBJECT *, long);
extern void VerticalScrollbar_cb(FL_OBJECT *, long);
extern void ScaleChoice_cb(FL_OBJECT *, long);
extern void ModeChoice_cb(FL_OBJECT *, long);
extern void FileMenu_cb(FL_OBJECT *, long);
extern void ComponentMenu_cb(FL_OBJECT *, long);
extern void PerformanceMenu_cb(FL_OBJECT *, long);
extern void MapsMenu_cb(FL_OBJECT *, long);
extern void AlignMenu_cb(FL_OBJECT *, long);
extern void UtilitiesMenu_cb(FL_OBJECT *, long);
extern void ScenariosMenu_cb(FL_OBJECT *, long);

extern void dummy_cb(FL_OBJECT *, long);
extern void PoolType_cb(FL_OBJECT *, long);
extern void CAOKButton_cb(FL_OBJECT *, long);
extern void CACancelButton_cb(FL_OBJECT *, long);
extern void StackButton_cb(FL_OBJECT *, long);
extern void ComponentType_cb(FL_OBJECT *, long);
extern void PoolPluginButton_cb(FL_OBJECT *, long);
extern void InstallExistingButton_cb(FL_OBJECT *, long);
extern void CreateNewButton_cb(FL_OBJECT *, long);
extern void ExistingComponentsBrowser_cb(FL_OBJECT *, long);
extern void Colour_cb(FL_OBJECT *, long);

extern void EROKButton_cb(FL_OBJECT *, long);
extern void ERCancelButton_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void ERAddDPButton_cb(FL_OBJECT *, long);
extern void EREditDPButton_cb(FL_OBJECT *, long);
extern void ERRemoveDPButton_cb(FL_OBJECT *, long);
extern void ERDataStoresButton_cb(FL_OBJECT *, long);
extern void ERServiceRequestButton_cb(FL_OBJECT *, long);
extern void ExistingRespBrowser_cb(FL_OBJECT *, long);
extern void IERButton_cb(FL_OBJECT *, long);
extern void CNRButton_cb(FL_OBJECT *, long);
extern void EditSettingsButton_cb(FL_OBJECT *, long);

extern void SDOKButton_cb(FL_OBJECT *, long);
extern void SDCancelButton_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);

extern void PrintPrintButton_cb(FL_OBJECT *, long);
extern void PrintCancelButton_cb(FL_OBJECT *, long);
extern void PrintPrinttoFileButton_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void Date_cb(FL_OBJECT *, long);
extern void Time_cb(FL_OBJECT *, long);


extern void dummy_cb(FL_OBJECT *, long);
extern void DesignData_OKButton_cb(FL_OBJECT *, long);
extern void DesignData_CancelButton_cb(FL_OBJECT *, long);

extern void dummy_cb(FL_OBJECT *, long);
extern void StubLabelOKButton_cb(FL_OBJECT *, long);
extern void StubLabelCancelButton_cb(FL_OBJECT *, long);
extern void ContinuityBrowser_cb(FL_OBJECT *, long);
extern void InputOutputBrowsers_cb(FL_OBJECT *, long);
extern void Unbind_cb(FL_OBJECT *, long);
extern void Bind_cb(FL_OBJECT *, long);
extern void SDServiceRequestButton_cb(FL_OBJECT *, long);

extern void PluginSelector_cb(FL_OBJECT *, long);
extern void PluginChoiceOKButton_cb(FL_OBJECT *, long);
extern void BindPluginButton_cb(FL_OBJECT *, long);
extern void PICCancelButton_cb(FL_OBJECT *, long);
extern void PICRenameButton_cb(FL_OBJECT *, long);
extern void PICEditConditionButton_cb(FL_OBJECT *, long);
extern void Probability_cb(FL_OBJECT *, long);

extern void UnbindButton_cb(FL_OBJECT *, long);
extern void StubBindingOKButton_cb(FL_OBJECT *, long);
extern void EntryPointsBindButton_cb(FL_OBJECT *, long);
extern void ExitPointsBindButton_cb(FL_OBJECT *, long);
extern void StubBindingsList_cb(FL_OBJECT *, long);
extern void StubEntryPointsBrowser_cb(FL_OBJECT *, long);
extern void PluginEntryPointsBrowser_cb(FL_OBJECT *, long);
extern void PluginExitPointsBrowser_cb(FL_OBJECT *, long);
extern void StubExitPointsBrowser_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);

extern void dummy_cb(FL_OBJECT *, long);
extern void TimestampCreationOKButton_cb(FL_OBJECT *, long);

extern void ResponseTimeResult_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);

extern void ResponseTimesBrowser_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void VRTRemoveDialogButton_cb(FL_OBJECT *, long);
extern void VRTAcceptButton_cb(FL_OBJECT *, long);
extern void VRTCancelButton_cb(FL_OBJECT *, long);
extern void VRTDeleteButton_cb(FL_OBJECT *, long);

extern void DeviceBrowser_cb(FL_OBJECT *, long);
extern void DeviceTypeChoice_cb(FL_OBJECT *, long);
extern void AddDeviceButton_cb(FL_OBJECT *, long);
extern void DDRemoveDialogButton_cb(FL_OBJECT *, long);
extern void DDDeleteButton_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void DDAcceptButton_cb(FL_OBJECT *, long);
extern void DDCancelButton_cb(FL_OBJECT *, long);

extern void dummy_cb(FL_OBJECT *, long);
extern void TSRAcceptButton_cb(FL_OBJECT *, long);
extern void TSRCancelButton_cb(FL_OBJECT *, long);
extern void TSRDeleteButton_cb(FL_OBJECT *, long);
extern void TSRRemoveDialogButton_cb(FL_OBJECT *, long);
extern void TSBrowser_cb(FL_OBJECT *, long);

extern void DSChoice_cb(FL_OBJECT *, long);
extern void DSBrowser_cb(FL_OBJECT *, long);
extern void DSAddButton_cb(FL_OBJECT *, long);
extern void DSDeleteButton_cb(FL_OBJECT *, long);
extern void DSRemoveDialog_cb(FL_OBJECT *, long);
extern void DSAcceptButton_cb(FL_OBJECT *, long);
extern void DSCancelButton_cb(FL_OBJECT *, long);

extern void DUBrowser_cb(FL_OBJECT *, long);
extern void AvailableData_cb(FL_OBJECT *, long);
extern void AccessModes_cb(FL_OBJECT *, long);
extern void DURemoveDialogButton_cb(FL_OBJECT *, long);
extern void DUAddButton_cb(FL_OBJECT *, long);
extern void DUDeleteButton_cb(FL_OBJECT *, long);

extern void dummy_cb(FL_OBJECT *, long);
extern void APOKButton_cb(FL_OBJECT *, long);
extern void APCancelButton_cb(FL_OBJECT *, long);
extern void WorkloadType_cb(FL_OBJECT *, long);
extern void APExponentialButton_cb(FL_OBJECT *, long);
extern void APErlangButton_cb(FL_OBJECT *, long);
extern void APExpertButton_cb(FL_OBJECT *, long);
extern void APDeterministicButton_cb(FL_OBJECT *, long);
extern void APUniformButton_cb(FL_OBJECT *, long);

extern void PoolPluginSelector_cb(FL_OBJECT *, long);
extern void PoolPluginChoiceCancelButton_cb(FL_OBJECT *, long);
extern void PoolPluginChoiceOKButton_cb(FL_OBJECT *, long);
extern void PoolPluginChoiceRenameButton_cb(FL_OBJECT *, long);

extern void dummy_cb(FL_OBJECT *, long);
extern void GoalConditions_cb(FL_OBJECT *, long);
extern void GoalAcceptButton_cb(FL_OBJECT *, long);
extern void GoalCancelButton_cb(FL_OBJECT *, long);
extern void GoalConditionsAdd_cb(FL_OBJECT *, long);
extern void GoalConditionsEdit_cb(FL_OBJECT *, long);
extern void GoalConditionsDelete_cb(FL_OBJECT *, long);

extern void SRAcceptButton_cb(FL_OBJECT *, long);
extern void SRDeleteButton_cb(FL_OBJECT *, long);
extern void SRRemoveDialogButton_cb(FL_OBJECT *, long);
extern void SRCancelButton_cb(FL_OBJECT *, long);
extern void ServiceCategoryChoice_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void SRBrowser_cb(FL_OBJECT *, long);

extern void SelectMapBrowser_cb(FL_OBJECT *, long);
extern void SMInstallButton_cb(FL_OBJECT *, long);
extern void SMDeleteButton_cb(FL_OBJECT *, long);
extern void SMCancelButton_cb(FL_OBJECT *, long);
extern void SMRenameButton_cb(FL_OBJECT *, long);
extern void SMMapTypeChoice_cb(FL_OBJECT *, long);
extern void MapName_cb(FL_OBJECT *, long);
extern void ConvertTypeButton_cb(FL_OBJECT *, long);

extern void dummy_cb(FL_OBJECT *, long);
extern void HelpCloseButton_cb(FL_OBJECT *, long);

extern void ParentListBrowser_cb(FL_OBJECT *, long);
extern void PML_Dismiss_cb(FL_OBJECT *, long);
extern void PML_Install_cb(FL_OBJECT *, long);

extern void GenerateButton_cb(FL_OBJECT *, long);
extern void CancelGenerate_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);

extern void RemainingMapsBrowser_cb(FL_OBJECT *, long);
extern void SetMapsBrowser_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void SetPrimaryMapButton_cb(FL_OBJECT *, long);
extern void IncludeMapButton_cb(FL_OBJECT *, long);
extern void RemoveMapButton_cb(FL_OBJECT *, long);
extern void USSRemoveButton_cb(FL_OBJECT *, long);
extern void MapTypeChoice_cb(FL_OBJECT *, long);
extern void USSAcceptButton_cb(FL_OBJECT *, long);

extern void UcmSetBrowser_cb(FL_OBJECT *, long);
extern void USCancelButton_cb(FL_OBJECT *, long);
extern void RestrictNavigation_cb(FL_OBJECT *, long);
extern void EditSetButton_cb(FL_OBJECT *, long);
extern void DeleteSetButton_cb(FL_OBJECT *, long);

extern void ReplacingReference_cb(FL_OBJECT *, long);
extern void ReplaceList_cb(FL_OBJECT *, long);
extern void Deselect_cb(FL_OBJECT *, long);
extern void Replace_cb(FL_OBJECT *, long);
extern void Dismiss_cb(FL_OBJECT *, long);

extern void UCMSetsList_cb(FL_OBJECT *, long);
extern void AddCurrentMapButton_cb(FL_OBJECT *, long);
extern void RemoveCurrentMapButton_cb(FL_OBJECT *, long);
extern void RemoveAMSDialog_cb(FL_OBJECT *, long);



extern void BranchChoice_cb(FL_OBJECT *, long);
extern void BranchSelection_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void FSAcceptButton_cb(FL_OBJECT *, long);
extern void FSCancelButton_cb(FL_OBJECT *, long);
extern void FSEditConditionButton_cb(FL_OBJECT *, long);

extern void BooleanList_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void BVAcceptButton_cb(FL_OBJECT *, long);
extern void BVAddButton_cb(FL_OBJECT *, long);
extern void BVDeleteButton_cb(FL_OBJECT *, long);
extern void BVCancelButton_cb(FL_OBJECT *, long);
extern void BVRemoveDialogButton_cb(FL_OBJECT *, long);

extern void GroupsList_cb(FL_OBJECT *, long);
extern void DeleteGroupButton_cb(FL_OBJECT *, long);
extern void ScenariosList_cb(FL_OBJECT *, long);
extern void EditScenarioButton_cb(FL_OBJECT *, long);
extern void DeleteScenarioButton_cb(FL_OBJECT *, long);
extern void VSRemoveDialogButton_cb(FL_OBJECT *, long);
extern void AddScenarioButton_cb(FL_OBJECT *, long);
extern void GroupMSCButton_cb(FL_OBJECT *, long);
extern void AddGroupButton_cb(FL_OBJECT *, long);
extern void EditGroupButton_cb(FL_OBJECT *, long);
extern void VariableInitializations_cb(FL_OBJECT *, long);
extern void dummy_cb(FL_OBJECT *, long);
extern void DeleteInitButton_cb(FL_OBJECT *, long);
extern void ToggleInitButton_cb(FL_OBJECT *, long);
extern void AddInitButton_cb(FL_OBJECT *, long);
extern void StartingPoints_cb(FL_OBJECT *, long);
extern void ChangeGroupButton_cb(FL_OBJECT *, long);
extern void EditStartList_cb(FL_OBJECT *, long);
extern void AddSelectedGroupButton_cb(FL_OBJECT *, long);
extern void ViewBVButton_cb(FL_OBJECT *, long);
extern void InitializationType_cb(FL_OBJECT *, long);
extern void PostconditionType_cb(FL_OBJECT *, long);
extern void GroupXMLButton_cb(FL_OBJECT *, long);
extern void ScenarioXMLButton_cb(FL_OBJECT *, long);
extern void ScenarioMSCButton_cb(FL_OBJECT *, long);
extern void TraceButton_cb(FL_OBJECT *, long);
extern void Duplicate_cb(FL_OBJECT *, long);


extern void ScenarioList_cb(FL_OBJECT *, long);
extern void HighlightPath_cb(FL_OBJECT *, long);
extern void Generate_cb(FL_OBJECT *, long);
extern void GenerateXML_cb(FL_OBJECT *, long);
extern void LSSClose_cb(FL_OBJECT *, long);


extern void ClearLogButton_cb(FL_OBJECT *, long);
extern void DismissButton_cb(FL_OBJECT *, long);

extern void OperationsList_cb(FL_OBJECT *, long);
extern void UBVList_cb(FL_OBJECT *, long);
extern void TrueButton_cb(FL_OBJECT *, long);
extern void FalseButton_cb(FL_OBJECT *, long);
extern void ExpressionButton_cb(FL_OBJECT *, long);
extern void EditExpression_cb(FL_OBJECT *, long);
extern void VariableOperationsClose_cb(FL_OBJECT *, long);
extern void AddOperation_cb(FL_OBJECT *, long);
extern void DeleteOperation_cb(FL_OBJECT *, long);

extern void ScenarioStartList_cb(FL_OBJECT *, long);
extern void MapsList_cb(FL_OBJECT *, long);
extern void PathStartList_cb(FL_OBJECT *, long);
extern void SPClose_cb(FL_OBJECT *, long);
extern void RemoveStart_cb(FL_OBJECT *, long);
extern void AddStart_cb(FL_OBJECT *, long);
extern void Lower_cb(FL_OBJECT *, long);
extern void Raise_cb(FL_OBJECT *, long);


/**** Forms and Objects ****/
typedef struct {
	FL_FORM *Main;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *DrawingArea;
	FL_OBJECT *Responsibilities;
	FL_OBJECT *Title;
	FL_OBJECT *Description;
	FL_OBJECT *Tools;
	FL_OBJECT *ComponentButton;
	FL_OBJECT *PathButton;
	FL_OBJECT *SelectButton;
	FL_OBJECT *DecompositionLevel;
	FL_OBJECT *ConditionControls;
	FL_OBJECT *Conditions1_Delete;
	FL_OBJECT *Conditions1_Edit;
	FL_OBJECT *Conditions2_Delete;
	FL_OBJECT *Conditions2_Edit;
	FL_OBJECT *EditResponsibility;
	FL_OBJECT *ConditionAddControls;
	FL_OBJECT *Conditions1;
	FL_OBJECT *Conditions1_Add;
	FL_OBJECT *Conditions2;
	FL_OBJECT *Conditions2_Add;
	FL_OBJECT *AboutMenu;
	FL_OBJECT *HorizontalScrollbar;
	FL_OBJECT *VerticalScrollbar;
	FL_OBJECT *ScaleChoice;
	FL_OBJECT *ModeChoice;
	FL_OBJECT *NavigationMode;
	FL_OBJECT *FileMenu;
	FL_OBJECT *ComponentMenu;
	FL_OBJECT *OptionsMenu;
	FL_OBJECT *PerformanceMenu;
	FL_OBJECT *MapsMenu;
	FL_OBJECT *AlignMenu;
	FL_OBJECT *UtilitiesMenu;
	FL_OBJECT *ScenariosMenu;
} FD_Main;

extern FD_Main * create_form_Main(void);
typedef struct {
	FL_FORM *CompAttrPopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ReplicationFactor;
	FL_OBJECT *PoolComponentType;
	FL_OBJECT *PoolType;
	FL_OBJECT *CAOKButton;
	FL_OBJECT *CACancelButton;
	FL_OBJECT *StackButton;
	FL_OBJECT *ComponentType;
	FL_OBJECT *ProtectedButton;
	FL_OBJECT *DynamicButton;
	FL_OBJECT *ActualGroup;
	FL_OBJECT *ActualButton;
	FL_OBJECT *FormalButton;
	FL_OBJECT *AnchoredGroup;
	FL_OBJECT *AnchoredButton;
	FL_OBJECT *NonAnchoredButton;
	FL_OBJECT *PoolPluginButton;
	FL_OBJECT *FixedButton;
	FL_OBJECT *InstallExistingButton;
	FL_OBJECT *CreateNewButton;
	FL_OBJECT *ExistingComponentsBrowser;
	FL_OBJECT *ColourGroup;
	FL_OBJECT *ColourButton17;
	FL_OBJECT *ColourButton0;
	FL_OBJECT *ColourButton1;
	FL_OBJECT *ColourButton2;
	FL_OBJECT *ColourButton3;
	FL_OBJECT *ColourButton4;
	FL_OBJECT *ColourButton5;
	FL_OBJECT *ColourButton6;
	FL_OBJECT *ColourButton7;
	FL_OBJECT *ColourButton8;
	FL_OBJECT *ColourButton9;
	FL_OBJECT *ColourButton10;
	FL_OBJECT *ColourButton11;
	FL_OBJECT *ColourButton12;
	FL_OBJECT *ColourButton13;
	FL_OBJECT *ColourButton14;
	FL_OBJECT *ColourButton15;
	FL_OBJECT *ColourButton16;
	FL_OBJECT *ComponentRole;
	FL_OBJECT *ColourDisplay;
	FL_OBJECT *Label;
	FL_OBJECT *ProcessorList;
} FD_CompAttrPopup;

extern FD_CompAttrPopup * create_form_CompAttrPopup(void);
typedef struct {
	FL_FORM *EditRespPopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *EROKButton;
	FL_OBJECT *ERCancelButton;
	FL_OBJECT *Name;
	FL_OBJECT *Description;
	FL_OBJECT *DirectionButtons;
	FL_OBJECT *RERightButton;
	FL_OBJECT *REUpButton;
	FL_OBJECT *RELeftButton;
	FL_OBJECT *REDownButton;
	FL_OBJECT *ExecutionSequence;
	FL_OBJECT *ERAddDPButton;
	FL_OBJECT *EREditDPButton;
	FL_OBJECT *ERRemoveDPButton;
	FL_OBJECT *DataStoresButton;
	FL_OBJECT *ServiceRequestsButton;
	FL_OBJECT *EditingModeBox;
	FL_OBJECT *ExistingRespBrowser;
	FL_OBJECT *ModeButtons;
	FL_OBJECT *IERButton;
	FL_OBJECT *CNRButton;
	FL_OBJECT *VariableSettings;
	FL_OBJECT *EditSettingsButton;
} FD_EditRespPopup;

extern FD_EditRespPopup * create_form_EditRespPopup(void);
typedef struct {
	FL_FORM *StructDynPopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *SDOKButton;
	FL_OBJECT *SDCancelButton;
	FL_OBJECT *DirectionButtons;
	FL_OBJECT *RightButton;
	FL_OBJECT *UpButton;
	FL_OBJECT *LeftButton;
	FL_OBJECT *DownButton;
	FL_OBJECT *SDTypeButtons;
	FL_OBJECT *MoveButton;
	FL_OBJECT *MoveStayButton;
	FL_OBJECT *CreateButton;
	FL_OBJECT *DestroyButton;
	FL_OBJECT *CopyButton;
	FL_OBJECT *PathButtons;
	FL_OBJECT *IntoPathButton;
	FL_OBJECT *OutofPathButton;
	FL_OBJECT *LengthSlider;
	FL_OBJECT *SourcePool;
} FD_StructDynPopup;

extern FD_StructDynPopup * create_form_StructDynPopup(void);
typedef struct {
	FL_FORM *PrintPopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *PrintPrintButton;
	FL_OBJECT *PrintCancelButton;
	FL_OBJECT *PrintPrinttoFileButton;
	FL_OBJECT *PrinterInput;
	FL_OBJECT *FlagBox;
	FL_OBJECT *MapDescription;
	FL_OBJECT *Responsibilities;
	FL_OBJECT *PathElements;
	FL_OBJECT *Goals;
	FL_OBJECT *StubDescriptions;
	FL_OBJECT *ComponentDescriptions;
	FL_OBJECT *LayoutOptions;
	FL_OBJECT *NewPage;
	FL_OBJECT *CenterHeadings;
	FL_OBJECT *Date;
	FL_OBJECT *Time;
	FL_OBJECT *DesignDescription;
	FL_OBJECT *Performance;
	FL_OBJECT *ParentListing;
	FL_OBJECT *UCMSets;
	FL_OBJECT *MapList;
	FL_OBJECT *AllMaps;
	FL_OBJECT *UCMSet;
	FL_OBJECT *Current;
	FL_OBJECT *CurrentSubtree;
	FL_OBJECT *FontSize;
	FL_OBJECT *RegularFonts;
	FL_OBJECT *LargeFonts;
	FL_OBJECT *Scenarios;
} FD_PrintPopup;

extern FD_PrintPopup * create_form_PrintPopup(void);
typedef struct {
	FL_FORM *StringParagraphDialog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *String;
	FL_OBJECT *Paragraph;
	FL_OBJECT *AcceptButton;
	FL_OBJECT *CancelButton;
} FD_StringParagraphDialog;

extern FD_StringParagraphDialog * create_form_StringParagraphDialog(void);
typedef struct {
	FL_FORM *DesignData;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *DesignName;
	FL_OBJECT *DesignDescription;
	FL_OBJECT *DesignData_OKButton;
	FL_OBJECT *DesignData_CancelButton;
} FD_DesignData;

extern FD_DesignData * create_form_DesignData(void);
typedef struct {
	FL_FORM *StubCreationPopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *StubLabel;
	FL_OBJECT *AcceptButton;
	FL_OBJECT *StubTypeButtons;
	FL_OBJECT *StaticButton;
	FL_OBJECT *DynamicButton;
	FL_OBJECT *CancelButton;
	FL_OBJECT *SharedButton;
	FL_OBJECT *SelectionPolicy;
	FL_OBJECT *ContinuityBrowser;
	FL_OBJECT *InputsBrowser;
	FL_OBJECT *OutputsBrowser;
	FL_OBJECT *Unbind;
	FL_OBJECT *Bind;
	FL_OBJECT *ServiceRequests;
} FD_StubCreationPopup;

extern FD_StubCreationPopup * create_form_StubCreationPopup(void);
typedef struct {
	FL_FORM *PluginChoicePopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *PluginSelector;
	FL_OBJECT *PluginChoiceOKButton;
	FL_OBJECT *BindPluginButton;
	FL_OBJECT *PICCancelButton;
	FL_OBJECT *RenameButton;
	FL_OBJECT *SelectionPolicy;
	FL_OBJECT *LogicalCondition;
	FL_OBJECT *PICEditConditionButton;
	FL_OBJECT *Probability;
} FD_PluginChoicePopup;

extern FD_PluginChoicePopup * create_form_PluginChoicePopup(void);
typedef struct {
	FL_FORM *StubBindingPopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *UnbindButton;
	FL_OBJECT *StubBindingOKButton;
	FL_OBJECT *EntryPointsBindButton;
	FL_OBJECT *ExitPointsBindButton;
	FL_OBJECT *StubBindingsList;
	FL_OBJECT *StubEntryPointsBrowser;
	FL_OBJECT *PluginEntryPointsBrowser;
	FL_OBJECT *PluginExitPointsBrowser;
	FL_OBJECT *StubExitPointsBrowser;
	FL_OBJECT *PluginName;
	FL_OBJECT *EnforcedBindings;
} FD_StubBindingPopup;

extern FD_StubBindingPopup * create_form_StubBindingPopup(void);
typedef struct {
	FL_FORM *TimestampCreationPopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *TimestampLabel;
	FL_OBJECT *TimestampDirectionButons;
	FL_OBJECT *UpButton;
	FL_OBJECT *DownButton;
	FL_OBJECT *LeftButton;
	FL_OBJECT *RightButton;
	FL_OBJECT *TimestampCreationOKButton;
	FL_OBJECT *ReferenceButtons;
	FL_OBJECT *PreviousButton;
	FL_OBJECT *NextButton;
} FD_TimestampCreationPopup;

extern FD_TimestampCreationPopup * create_form_TimestampCreationPopup(void);
typedef struct {
	FL_FORM *ResponseTimePopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ResponseTimeOKButton;
	FL_OBJECT *ResponseTimeCancelButton;
	FL_OBJECT *ResponseTimeValue;
	FL_OBJECT *ResponseTimePrompt;
	FL_OBJECT *ResponseTimePercentage;
	FL_OBJECT *Name;
} FD_ResponseTimePopup;

extern FD_ResponseTimePopup * create_form_ResponseTimePopup(void);
typedef struct {
	FL_FORM *ViewResponseTimes;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ResponseTimesBrowser;
	FL_OBJECT *ResponseTimeValue;
	FL_OBJECT *ResponseTimePercentage;
	FL_OBJECT *VRTRemoveDialogButton;
	FL_OBJECT *EditRTButtons;
	FL_OBJECT *VRTAcceptButton;
	FL_OBJECT *VRTCancelButton;
	FL_OBJECT *VRTDeleteButton;
	FL_OBJECT *Name;
} FD_ViewResponseTimes;

extern FD_ViewResponseTimes * create_form_ViewResponseTimes(void);
typedef struct {
	FL_FORM *DeviceDirectory;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *DeviceBrowser;
	FL_OBJECT *DeviceTypeChoice;
	FL_OBJECT *AddDeviceButton;
	FL_OBJECT *DDRemoveDialogButton;
	FL_OBJECT *DDDeleteButton;
	FL_OBJECT *EditDevice;
	FL_OBJECT *EditDeviceBox;
	FL_OBJECT *DeviceNameInput;
	FL_OBJECT *DDAcceptButton;
	FL_OBJECT *DDCancelButton;
	FL_OBJECT *DeviceCharacteristics;
	FL_OBJECT *OpTime;
} FD_DeviceDirectory;

extern FD_DeviceDirectory * create_form_DeviceDirectory(void);
typedef struct {
	FL_FORM *ViewTimestampRT;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ResponseTimeValue;
	FL_OBJECT *ResponseTimePercentage;
	FL_OBJECT *EditRTButtons;
	FL_OBJECT *TSRAcceptButton;
	FL_OBJECT *TSRCancelButton;
	FL_OBJECT *TSRDeleteButton;
	FL_OBJECT *TSRRemoveDialogButton;
	FL_OBJECT *TerminatingTSBrowser;
	FL_OBJECT *StartingTSBrowser;
	FL_OBJECT *Prompt;
	FL_OBJECT *Name;
} FD_ViewTimestampRT;

extern FD_ViewTimestampRT * create_form_ViewTimestampRT(void);
typedef struct {
	FL_FORM *DataStores;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *DataStoreChoice;
	FL_OBJECT *DSBrowser;
	FL_OBJECT *DSAddButton;
	FL_OBJECT *DSDeleteButton;
	FL_OBJECT *DSRemoveDialog;
	FL_OBJECT *InputFields;
	FL_OBJECT *DSInput;
	FL_OBJECT *DSCancelButton;
	FL_OBJECT *DSAcceptButton;
} FD_DataStores;

extern FD_DataStores * create_form_DataStores(void);
typedef struct {
	FL_FORM *DataStoreUsage;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *Selections;
	FL_OBJECT *DUBrowser;
	FL_OBJECT *AvailableData;
	FL_OBJECT *AccessModes;
	FL_OBJECT *Prompt;
	FL_OBJECT *DURemoveDialogButton;
	FL_OBJECT *DUAddButton;
	FL_OBJECT *DUDeleteButton;
} FD_DataStoreUsage;

extern FD_DataStoreUsage * create_form_DataStoreUsage(void);
typedef struct {
	FL_FORM *ArrivalProcess;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ExpertInput;
	FL_OBJECT *DistributionBox;
	FL_OBJECT *Input1;
	FL_OBJECT *Input2;
	FL_OBJECT *APOKButton;
	FL_OBJECT *APCancelButton;
	FL_OBJECT *WorkloadType;
	FL_OBJECT *PopulationSize;
	FL_OBJECT *DistributionButtons;
	FL_OBJECT *APExponentialButton;
	FL_OBJECT *APErlangButton;
	FL_OBJECT *APExpertButton;
	FL_OBJECT *APDeterministicButton;
	FL_OBJECT *APUniformButton;
	FL_OBJECT *DistributionLabel;
} FD_ArrivalProcess;

extern FD_ArrivalProcess * create_form_ArrivalProcess(void);
typedef struct {
	FL_FORM *PoolPluginChoicePopup;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *PoolPluginSelector;
	FL_OBJECT *PoolPluginChoiceCancelButton;
	FL_OBJECT *PoolPluginChoiceOKButton;
	FL_OBJECT *PoolPluginChoiceRenameButton;
} FD_PoolPluginChoicePopup;

extern FD_PoolPluginChoicePopup * create_form_PoolPluginChoicePopup(void);
typedef struct {
	FL_FORM *GoalEditor;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *Label;
	FL_OBJECT *GoalName;
	FL_OBJECT *GoalDescription;
	FL_OBJECT *GoalPreconditions;
	FL_OBJECT *GoalPostconditions;
	FL_OBJECT *GoalAcceptButton;
	FL_OBJECT *GoalCancelButton;
	FL_OBJECT *GoalPostconditionsAdd;
	FL_OBJECT *GoalPostconditionsEdit;
	FL_OBJECT *GoalPostconditionsDelete;
	FL_OBJECT *GoalPreconditionsAdd;
	FL_OBJECT *GoalPreconditionsEdit;
	FL_OBJECT *GoalPreconditionsDelete;
} FD_GoalEditor;

extern FD_GoalEditor * create_form_GoalEditor(void);
typedef struct {
	FL_FORM *ServiceRequests;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *SRAcceptButton;
	FL_OBJECT *SRDeleteButton;
	FL_OBJECT *SRRemoveDialogButton;
	FL_OBJECT *SRCancelButton;
	FL_OBJECT *ServiceCategoryChoice;
	FL_OBJECT *RequestNumberInput;
	FL_OBJECT *SRBrowser;
	FL_OBJECT *ServiceType;
} FD_ServiceRequests;

extern FD_ServiceRequests * create_form_ServiceRequests(void);
typedef struct {
	FL_FORM *SelectMap;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *SelectMapBrowser;
	FL_OBJECT *InstallButton;
	FL_OBJECT *DeleteButton;
	FL_OBJECT *CancelButton;
	FL_OBJECT *RenameButton;
	FL_OBJECT *MapTypeChoice;
	FL_OBJECT *MapName;
	FL_OBJECT *ConvertTypeButton;
} FD_SelectMap;

extern FD_SelectMap * create_form_SelectMap(void);
typedef struct {
	FL_FORM *HelpWindow;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *HelpDisplay;
	FL_OBJECT *HelpCloseButton;
} FD_HelpWindow;

extern FD_HelpWindow * create_form_HelpWindow(void);
typedef struct {
	FL_FORM *ParentMapList;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ParentListBrowser;
	FL_OBJECT *DismissButton;
	FL_OBJECT *InstallButton;
} FD_ParentMapList;

extern FD_ParentMapList * create_form_ParentMapList(void);
typedef struct {
	FL_FORM *GraphicCreationDialog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ChoicesBox;
	FL_OBJECT *GenerateButton;
	FL_OBJECT *CancelGenerate;
	FL_OBJECT *MapGroup;
	FL_OBJECT *CurrentMap;
	FL_OBJECT *CurrentSubtree;
	FL_OBJECT *AllMaps;
	FL_OBJECT *UCMSet;
} FD_GraphicCreationDialog;

extern FD_GraphicCreationDialog * create_form_GraphicCreationDialog(void);
typedef struct {
	FL_FORM *UcmSetSpecification;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *RemainingMapsBrowser;
	FL_OBJECT *SetMapsBrowser;
	FL_OBJECT *SetNameInput;
	FL_OBJECT *PrimaryMapDisplay;
	FL_OBJECT *SetPrimaryMapButton;
	FL_OBJECT *IncludeMapButton;
	FL_OBJECT *RemoveMapButton;
	FL_OBJECT *MapCommentInput;
	FL_OBJECT *SetDescriptionInput;
	FL_OBJECT *USSRemoveButton;
	FL_OBJECT *MapTypeChoice;
	FL_OBJECT *USSAcceptButton;
} FD_UcmSetSpecification;

extern FD_UcmSetSpecification * create_form_UcmSetSpecification(void);
typedef struct {
	FL_FORM *UcmSetChoice;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *UcmSetBrowser;
	FL_OBJECT *USCancelButton;
	FL_OBJECT *ModificationButtons;
	FL_OBJECT *RestrictNavigation;
	FL_OBJECT *EditSetButton;
	FL_OBJECT *DeleteSetButton;
} FD_UcmSetChoice;

extern FD_UcmSetChoice * create_form_UcmSetChoice(void);
typedef struct {
	FL_FORM *ReferenceReplacement;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ReplacingReference;
	FL_OBJECT *ReplaceList;
	FL_OBJECT *Deselect;
	FL_OBJECT *Replace;
	FL_OBJECT *Dismiss;
} FD_ReferenceReplacement;

extern FD_ReferenceReplacement * create_form_ReferenceReplacement(void);
typedef struct {
	FL_FORM *AddCurrentMapUCMSet;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *CurrentMapDisplay;
	FL_OBJECT *UCMSetsList;
	FL_OBJECT *AddCurrentMapButton;
	FL_OBJECT *RemoveCurrentMapButton;
	FL_OBJECT *RemoveAMSDialog;
} FD_AddCurrentMapUCMSet;

extern FD_AddCurrentMapUCMSet * create_form_AddCurrentMapUCMSet(void);
typedef struct {
	FL_FORM *TwoStringDialog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *String1;
	FL_OBJECT *String2;
	FL_OBJECT *AcceptButton;
	FL_OBJECT *CancelButton;
} FD_TwoStringDialog;

extern FD_TwoStringDialog * create_form_TwoStringDialog(void);
typedef struct {
	FL_FORM *QuestionDialog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *YesButton;
	FL_OBJECT *NoButton;
	FL_OBJECT *String1;
	FL_OBJECT *String2;
} FD_QuestionDialog;

extern FD_QuestionDialog * create_form_QuestionDialog(void);
typedef struct {
	FL_FORM *BranchSpecification;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *BranchChoice;
	FL_OBJECT *BranchSelection;
	FL_OBJECT *Probability;
	FL_OBJECT *ForkName;
	FL_OBJECT *FSAcceptButton;
	FL_OBJECT *FSCancelButton;
	FL_OBJECT *LogicalCondition;
	FL_OBJECT *FSEditConditionButton;
} FD_BranchSpecification;

extern FD_BranchSpecification * create_form_BranchSpecification(void);
typedef struct {
	FL_FORM *BooleanVariables;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *BooleanList;
	FL_OBJECT *CurrentBoolean;
	FL_OBJECT *BVAcceptButton;
	FL_OBJECT *BVAddButton;
	FL_OBJECT *BVDeleteButton;
	FL_OBJECT *BVCancelButton;
	FL_OBJECT *BVRemoveDialogButton;
} FD_BooleanVariables;

extern FD_BooleanVariables * create_form_BooleanVariables(void);
typedef struct {
	FL_FORM *ScenarioSpecification;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *GroupsBox;
	FL_OBJECT *DeleteGroupButton;
	FL_OBJECT *ScenariosBox;
	FL_OBJECT *ScenariosList;
	FL_OBJECT *EditScenarioButton;
	FL_OBJECT *DeleteScenarioButton;
	FL_OBJECT *RemoveDialogButton;
	FL_OBJECT *AddScenarioButton;
	FL_OBJECT *GroupMSCButton;
	FL_OBJECT *GroupsList;
	FL_OBJECT *AddGroupButton;
	FL_OBJECT *EditGroupButton;
	FL_OBJECT *VariableInitializations;
	FL_OBJECT *BooleanVariables;
	FL_OBJECT *DeleteInitButton;
	FL_OBJECT *ToggleInitButton;
	FL_OBJECT *AddInitButton;
	FL_OBJECT *ValueGroup;
	FL_OBJECT *TrueButton;
	FL_OBJECT *FalseButton;
	FL_OBJECT *StartingPoints;
	FL_OBJECT *ChangeGroupButton;
	FL_OBJECT *ScenarioDescription;
	FL_OBJECT *ScenarioGroupDescription;
	FL_OBJECT *EditStartList;
	FL_OBJECT *AddSelectedGroupButton;
	FL_OBJECT *ViewBVButton;
	FL_OBJECT *TypeGroup;
	FL_OBJECT *InitializationType;
	FL_OBJECT *PostconditionType;
	FL_OBJECT *GroupXMLButton;
	FL_OBJECT *ScenarioOperationGroup;
	FL_OBJECT *ScenarioXMLButton;
	FL_OBJECT *ScenarioMSCButton;
	FL_OBJECT *TraceButton;
	FL_OBJECT *Duplicate;
} FD_ScenarioSpecification;

extern FD_ScenarioSpecification * create_form_ScenarioSpecification(void);
typedef struct {
	FL_FORM *ConditionEditor;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *BooleanVariables;
	FL_OBJECT *Condition;
	FL_OBJECT *AcceptButton;
	FL_OBJECT *CancelButton;
} FD_ConditionEditor;

extern FD_ConditionEditor * create_form_ConditionEditor(void);
typedef struct {
	FL_FORM *StartScenarios;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ScenarioList;
	FL_OBJECT *HighlightPath;
	FL_OBJECT *Generate;
	FL_OBJECT *GenerateXML;
	FL_OBJECT *LSSClose;
} FD_StartScenarios;

extern FD_StartScenarios * create_form_StartScenarios(void);
typedef struct {
	FL_FORM *MSCGenerationStatus;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *GenerateButton;
	FL_OBJECT *CancelButton;
	FL_OBJECT *Count;
	FL_OBJECT *Time;
	FL_OBJECT *CurrentScenario;
	FL_OBJECT *CurrentNumberLabel;
	FL_OBJECT *ScenarioBasename;
	FL_OBJECT *BytesWritten;
} FD_MSCGenerationStatus;

extern FD_MSCGenerationStatus * create_form_MSCGenerationStatus(void);
typedef struct {
	FL_FORM *ErrorDialog;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ErrorLog;
	FL_OBJECT *ClearLogButton;
	FL_OBJECT *DismissButton;
} FD_ErrorDialog;

extern FD_ErrorDialog * create_form_ErrorDialog(void);
typedef struct {
	FL_FORM *VariableOperations;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *OperationsList;
	FL_OBJECT *UBVList;
	FL_OBJECT *ValueButtons;
	FL_OBJECT *TrueButton;
	FL_OBJECT *FalseButton;
	FL_OBJECT *ExpressionButton;
	FL_OBJECT *ExpressionDisplay;
	FL_OBJECT *EditExpression;
	FL_OBJECT *VariableOperationsClose;
	FL_OBJECT *AddOperation;
	FL_OBJECT *DeleteOperation;
} FD_VariableOperations;

extern FD_VariableOperations * create_form_VariableOperations(void);
typedef struct {
	FL_FORM *EditStartList;
	void *vdata;
	char *cdata;
	long  ldata;
	FL_OBJECT *ScenarioStartList;
	FL_OBJECT *MapsList;
	FL_OBJECT *PathStartList;
	FL_OBJECT *SPClose;
	FL_OBJECT *RemoveStart;
	FL_OBJECT *AddStart;
	FL_OBJECT *Lower;
	FL_OBJECT *Raise;
} FD_EditStartList;

extern FD_EditStartList * create_form_EditStartList(void);

#endif /* FD_Main_h_ */
