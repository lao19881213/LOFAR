#
# Generated by gen_LofarPackageList_cmake.sh at Fri Mar 19 10:47:38 UTC 2010
#
#                            ---- DO NOT EDIT ----
#
#  This file creates for each LOFAR package a variable containing the relative
#  path to its source directory. 
#
#  ATTENTION: This file must be included BEFORE the first call of the
#  lofar_package() macro; either directly, or indirectly.
#
#  NOTE: This file must be kept up-to-date when project directories are added,
#  moved, or deleted. Use gen_LofarPackageList_cmake.sh to do so.
#
if(NOT DEFINED LOFAR_PACKAGE_LIST_INCLUDED)
  set(LOFAR_PACKAGE_LIST_INCLUDED TRUE)
  set(LCS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS)
  set(CEP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP)
  set(RTCP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP)
  set(SAS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SAS)
  set(MAC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC)
  set(LCU_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCU)
  set(SubSystems_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SubSystems)
  set(ALC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/ACC/ALC)
  set(PLC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/ACC/PLC)
  set(AMCBase_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/AMC/AMCBase)
  set(AMCImpl_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/AMC/AMCImpl)
  set(ACC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/ACC)
  set(AMC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/AMC)
  set(ApplCommon_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/ApplCommon)
  set(Blob_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/Blob)
  set(Common_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/Common)
  set(pyparameterset_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/pyparameterset)
  set(pytools_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/pytools)
  set(Stream_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/Stream)
  set(Tools_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/Tools)
  set(Transport_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCS/Transport)
  set(Interface_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/Interface)
  set(FCNP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/FCNP)
  set(CNProc_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/CNProc)
  set(IONProc_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/IONProc)
  set(Storage_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/Storage)
  set(RTCPTools_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/RTCPTools)
  set(Run_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/Run)
  set(LofarStMan_SOURCE_DIR ${CMAKE_SOURCE_DIR}/RTCP/LofarStMan)
  set(MWImager_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/Imager/MWImager)
  set(LofarGridder_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/Imager/LofarGridder)
  set(DPPP_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/DP3/DPPP)
  set(AOFlagger_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/DP3/AOFlagger)
  set(SPW_Combine_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/DP3/SPWCombine)
  set(BBSKernel_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/Calibration/BBSKernel)
  set(BBSControl_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/Calibration/BBSControl)
  set(MWCommon_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/MWCommon)
  set(MS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/MS)
  set(ParmDB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/ParmDB)
  set(Calibration_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/Calibration)
  set(DP3_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/DP3)
  set(Imager_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/Imager)
  set(Pipeline_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/Pipeline)
  set(pyparmdb_SOURCE_DIR ${CMAKE_SOURCE_DIR}/CEP/pyparmdb)
  set(jOTDB2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SAS/OTB/jOTDB2)
  set(OTDB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SAS/OTDB)
  set(OTB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SAS/OTB)
  set(GCFTM_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/GCF/TM)
  set(GCFPVSS_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/GCF/PVSS)
  set(GCFRTDB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/GCF/RTDB)
  set(CAL_Protocol_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/CAL_Protocol)
  set(BS_Protocol_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/BS_Protocol)
  set(Cal_Server_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/Cal_Server)
  set(Beam_Server_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/Beam_Server)
  set(SHMInfo_Server_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/SHMInfo_Server)
  set(LBA_Calibration_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/LBA_Calibration)
  set(IBS_Protocol_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/IBS_Protocol)
  set(ICAL_Protocol_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/ICAL_Protocol)
  set(ITRFBeamServer_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/ITRFBeamServer)
  set(ITRFCalServer_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC/ITRFCalServer)
  set(RSP_Protocol_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PIC/RSP_Protocol)
  set(TBB_Protocol_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PIC/TBB_Protocol)
  set(RSP_Driver_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PIC/RSP_Driver)
  set(TBB_Driver_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PIC/TBB_Driver)
  set(APLCommon_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/APLCommon)
  set(RTCCommon_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/RTCCommon)
  set(RTDBCommon_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/RTDBCommon)
  set(TestSuite_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/TestSuite)
  set(CUDaemons_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/CUDaemons)
  set(CURTDBDaemons_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/CURTDBDaemons)
  set(PIC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PIC)
  set(PAC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/PAC)
  set(VHECR_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/VHECR)
  set(CEPCU_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/CEPCU)
  set(MainCU_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/MainCU)
  set(StationCU_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/StationCU)
  set(TestCtlr_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL/TestCtlr)
  set(MACIO_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/MACIO)
  set(GCF_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/GCF)
  set(APL_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/APL)
  set(Deployment_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/Deployment)
  set(Navigator2_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/Navigator2)
  set(MACTools_SOURCE_DIR ${CMAKE_SOURCE_DIR}/MAC/Tools)
  set(Firmware-Tools_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCU/Firmware/tools)
  set(Firmware_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCU/Firmware)
  set(StationTest_SOURCE_DIR ${CMAKE_SOURCE_DIR}/LCU/StationTest)
  set(CCU_MAC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SubSystems/CCU_MAC)
  set(LCU_MAC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SubSystems/LCU_MAC)
  set(MCU_MAC_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SubSystems/MCU_MAC)
  set(Offline_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SubSystems/Offline)
  set(SAS_OTDB_SOURCE_DIR ${CMAKE_SOURCE_DIR}/SubSystems/SAS_OTDB)
endif(NOT DEFINED LOFAR_PACKAGE_LIST_INCLUDED)
