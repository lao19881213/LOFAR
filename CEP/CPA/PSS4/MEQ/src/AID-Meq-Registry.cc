    // This file is generated automatically -- do not edit
    // Generated by /home/oms/LOFAR/autoconf_share/../DMI/aid/build_aid_maps.pl
    #include <DMI/AtomicID.h>
    #include <DMI/TypeInfo.h>
    #include <DMI/DynamicTypeManager.h>
    #include <DMI/Packer.h>
    
#include "Domain.h"
BlockableObject * __construct_MeqDomain (int n) { return n>0 ? new Meq::Domain [n] : new Meq::Domain; }
#include "Cells.h"
BlockableObject * __construct_MeqCells (int n) { return n>0 ? new Meq::Cells [n] : new Meq::Cells; }
#include "VellSet.h"
BlockableObject * __construct_MeqVellSet (int n) { return n>0 ? new Meq::VellSet [n] : new Meq::VellSet; }
#include "Result.h"
BlockableObject * __construct_MeqResult (int n) { return n>0 ? new Meq::Result [n] : new Meq::Result; }
#include "Polc.h"
BlockableObject * __construct_MeqPolc (int n) { return n>0 ? new Meq::Polc [n] : new Meq::Polc; }
#include "Node.h"
BlockableObject * __construct_MeqNode (int n) { return n>0 ? new Meq::Node [n] : new Meq::Node; }
#include "Function.h"
BlockableObject * __construct_MeqFunction (int n) { return n>0 ? new Meq::Function [n] : new Meq::Function; }
#include "Constant.h"
BlockableObject * __construct_MeqConstant (int n) { return n>0 ? new Meq::Constant [n] : new Meq::Constant; }
#include "Parm.h"
BlockableObject * __construct_MeqParm (int n) { return n>0 ? new Meq::Parm [n] : new Meq::Parm; }
#include "Freq.h"
BlockableObject * __construct_MeqFreq (int n) { return n>0 ? new Meq::Freq [n] : new Meq::Freq; }
#include "Time.h"
BlockableObject * __construct_MeqTime (int n) { return n>0 ? new Meq::Time [n] : new Meq::Time; }
#include "Selector.h"
BlockableObject * __construct_MeqSelector (int n) { return n>0 ? new Meq::Selector [n] : new Meq::Selector; }
#include "Composer.h"
BlockableObject * __construct_MeqComposer (int n) { return n>0 ? new Meq::Composer [n] : new Meq::Composer; }
#include "Add.h"
BlockableObject * __construct_MeqAdd (int n) { return n>0 ? new Meq::Add [n] : new Meq::Add; }
#include "Subtract.h"
BlockableObject * __construct_MeqSubtract (int n) { return n>0 ? new Meq::Subtract [n] : new Meq::Subtract; }
#include "Multiply.h"
BlockableObject * __construct_MeqMultiply (int n) { return n>0 ? new Meq::Multiply [n] : new Meq::Multiply; }
#include "Divide.h"
BlockableObject * __construct_MeqDivide (int n) { return n>0 ? new Meq::Divide [n] : new Meq::Divide; }
#include "Sin.h"
BlockableObject * __construct_MeqSin (int n) { return n>0 ? new Meq::Sin [n] : new Meq::Sin; }
#include "Cos.h"
BlockableObject * __construct_MeqCos (int n) { return n>0 ? new Meq::Cos [n] : new Meq::Cos; }
#include "Exp.h"
BlockableObject * __construct_MeqExp (int n) { return n>0 ? new Meq::Exp [n] : new Meq::Exp; }
#include "Pow.h"
BlockableObject * __construct_MeqPow (int n) { return n>0 ? new Meq::Pow [n] : new Meq::Pow; }
#include "Sqr.h"
BlockableObject * __construct_MeqSqr (int n) { return n>0 ? new Meq::Sqr [n] : new Meq::Sqr; }
#include "Sqrt.h"
BlockableObject * __construct_MeqSqrt (int n) { return n>0 ? new Meq::Sqrt [n] : new Meq::Sqrt; }
#include "Conj.h"
BlockableObject * __construct_MeqConj (int n) { return n>0 ? new Meq::Conj [n] : new Meq::Conj; }
#include "ToComplex.h"
BlockableObject * __construct_MeqToComplex (int n) { return n>0 ? new Meq::ToComplex [n] : new Meq::ToComplex; }
#include "UVW.h"
BlockableObject * __construct_MeqUVW (int n) { return n>0 ? new Meq::UVW [n] : new Meq::UVW; }
#include "Request.h"
BlockableObject * __construct_MeqRequest (int n) { return n>0 ? new Meq::Request [n] : new Meq::Request; }
#include "Condeq.h"
BlockableObject * __construct_MeqCondeq (int n) { return n>0 ? new Meq::Condeq [n] : new Meq::Condeq; }
#include "Solver.h"
BlockableObject * __construct_MeqSolver (int n) { return n>0 ? new Meq::Solver [n] : new Meq::Solver; }
  
    int aidRegistry_Meq ()
    {
      static int res = 

        AtomicID::registerId(-1233,"node")+
        AtomicID::registerId(-1248,"class")+
        AtomicID::registerId(-1188,"name")+
        AtomicID::registerId(-1060,"state")+
        AtomicID::registerId(-1226,"child")+
        AtomicID::registerId(-1220,"children")+
        AtomicID::registerId(-1210,"request")+
        AtomicID::registerId(-1228,"result")+
        AtomicID::registerId(-1368,"vellset")+
        AtomicID::registerId(-1250,"rider")+
        AtomicID::registerId(-1277,"command")+
        AtomicID::registerId(-1087,"id")+
        AtomicID::registerId(-1122,"group")+
        AtomicID::registerId(-1064,"add")+
        AtomicID::registerId(-1247,"cells")+
        AtomicID::registerId(-1213,"domain")+
        AtomicID::registerId(-1128,"freq")+
        AtomicID::registerId(-1152,"time")+
        AtomicID::registerId(-1252,"times")+
        AtomicID::registerId(-1362,"step")+
        AtomicID::registerId(-1363,"steps")+
        AtomicID::registerId(-1211,"calc")+
        AtomicID::registerId(-1230,"deriv")+
        AtomicID::registerId(-1370,"vells")+
        AtomicID::registerId(-1371,"vellsets")+
        AtomicID::registerId(-1324,"nodeindex")+
        AtomicID::registerId(-1361,"table")+
        AtomicID::registerId(-1231,"default")+
        AtomicID::registerId(-1256,"value")+
        AtomicID::registerId(-1051,"index")+
        AtomicID::registerId(-1177,"num")+
        AtomicID::registerId(-1375,"cache")+
        AtomicID::registerId(-1164,"code")+
        AtomicID::registerId(-1234,"parm")+
        AtomicID::registerId(-1254,"spid")+
        AtomicID::registerId(-1408,"coeff")+
        AtomicID::registerId(-1229,"perturbed")+
        AtomicID::registerId(-1218,"perturbations")+
        AtomicID::registerId(-1396,"names")+
        AtomicID::registerId(-1394,"pert")+
        AtomicID::registerId(-1393,"relative")+
        AtomicID::registerId(-1287,"mask")+
        AtomicID::registerId(-1245,"results")+
        AtomicID::registerId(-1036,"fail")+
        AtomicID::registerId(-1132,"origin")+
        AtomicID::registerId(-1359,"line")+
        AtomicID::registerId(-1045,"message")+
        AtomicID::registerId(-1364,"contagious")+
        AtomicID::registerId(-1395,"normalized")+
        AtomicID::registerId(-1366,"solvable")+
        AtomicID::registerId(-1373,"config")+
        AtomicID::registerId(-1374,"groups")+
        AtomicID::registerId(-1381,"all")+
        AtomicID::registerId(-1382,"by")+
        AtomicID::registerId(-1046,"list")+
        AtomicID::registerId(-1412,"polc")+
        AtomicID::registerId(-1383,"polcs")+
        AtomicID::registerId(-1405,"scale")+
        AtomicID::registerId(-1411,"dbid")+
        AtomicID::registerId(-1409,"grow")+
        AtomicID::registerId(-1410,"inf")+
        AtomicID::registerId(-1189,"weight")+
        AtomicID::registerId(-1385,"epsilon")+
        AtomicID::registerId(-1384,"usesvd")+
        AtomicID::registerId(-1316,"set")+
        AtomicID::registerId(-1273,"auto")+
        AtomicID::registerId(-1391,"save")+
        AtomicID::registerId(-1404,"metrics")+
        AtomicID::registerId(-1402,"rank")+
        AtomicID::registerId(-1403,"fit")+
        AtomicID::registerId(-1397,"errors")+
        AtomicID::registerId(-1401,"covar")+
        AtomicID::registerId(-1135,"flag")+
        AtomicID::registerId(-1400,"mu")+
        AtomicID::registerId(-1399,"stddev")+
        AtomicID::registerId(-1398,"chi")+
        AtomicID::registerId(-1235,"meqdomain")+
        TypeInfoReg::addToRegistry(-1235,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1235,__construct_MeqDomain)+
        AtomicID::registerId(-1237,"meqcells")+
        TypeInfoReg::addToRegistry(-1237,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1237,__construct_MeqCells)+
        AtomicID::registerId(-1369,"meqvellset")+
        TypeInfoReg::addToRegistry(-1369,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1369,__construct_MeqVellSet)+
        AtomicID::registerId(-1246,"meqresult")+
        TypeInfoReg::addToRegistry(-1246,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1246,__construct_MeqResult)+
        AtomicID::registerId(-1407,"meqpolc")+
        TypeInfoReg::addToRegistry(-1407,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1407,__construct_MeqPolc)+
        AtomicID::registerId(-1242,"meqnode")+
        TypeInfoReg::addToRegistry(-1242,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1242,__construct_MeqNode)+
        AtomicID::registerId(-1216,"meqfunction")+
        TypeInfoReg::addToRegistry(-1216,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1216,__construct_MeqFunction)+
        AtomicID::registerId(-1387,"meqconstant")+
        TypeInfoReg::addToRegistry(-1387,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1387,__construct_MeqConstant)+
        AtomicID::registerId(-1244,"meqparm")+
        TypeInfoReg::addToRegistry(-1244,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1244,__construct_MeqParm)+
        AtomicID::registerId(-1219,"meqfreq")+
        TypeInfoReg::addToRegistry(-1219,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1219,__construct_MeqFreq)+
        AtomicID::registerId(-1225,"meqtime")+
        TypeInfoReg::addToRegistry(-1225,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1225,__construct_MeqTime)+
        AtomicID::registerId(-1255,"meqselector")+
        TypeInfoReg::addToRegistry(-1255,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1255,__construct_MeqSelector)+
        AtomicID::registerId(-1241,"meqcomposer")+
        TypeInfoReg::addToRegistry(-1241,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1241,__construct_MeqComposer)+
        AtomicID::registerId(-1236,"meqadd")+
        TypeInfoReg::addToRegistry(-1236,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1236,__construct_MeqAdd)+
        AtomicID::registerId(-1232,"meqsubtract")+
        TypeInfoReg::addToRegistry(-1232,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1232,__construct_MeqSubtract)+
        AtomicID::registerId(-1227,"meqmultiply")+
        TypeInfoReg::addToRegistry(-1227,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1227,__construct_MeqMultiply)+
        AtomicID::registerId(-1223,"meqdivide")+
        TypeInfoReg::addToRegistry(-1223,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1223,__construct_MeqDivide)+
        AtomicID::registerId(-1224,"meqsin")+
        TypeInfoReg::addToRegistry(-1224,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1224,__construct_MeqSin)+
        AtomicID::registerId(-1243,"meqcos")+
        TypeInfoReg::addToRegistry(-1243,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1243,__construct_MeqCos)+
        AtomicID::registerId(-1240,"meqexp")+
        TypeInfoReg::addToRegistry(-1240,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1240,__construct_MeqExp)+
        AtomicID::registerId(-1214,"meqpow")+
        TypeInfoReg::addToRegistry(-1214,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1214,__construct_MeqPow)+
        AtomicID::registerId(-1249,"meqsqr")+
        TypeInfoReg::addToRegistry(-1249,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1249,__construct_MeqSqr)+
        AtomicID::registerId(-1251,"meqsqrt")+
        TypeInfoReg::addToRegistry(-1251,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1251,__construct_MeqSqrt)+
        AtomicID::registerId(-1212,"meqconj")+
        TypeInfoReg::addToRegistry(-1212,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1212,__construct_MeqConj)+
        AtomicID::registerId(-1217,"meqtocomplex")+
        TypeInfoReg::addToRegistry(-1217,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1217,__construct_MeqToComplex)+
        AtomicID::registerId(-1239,"mequvw")+
        TypeInfoReg::addToRegistry(-1239,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1239,__construct_MeqUVW)+
        AtomicID::registerId(-1380,"ra")+
        AtomicID::registerId(-1379,"dec")+
        AtomicID::registerId(-1376,"stx")+
        AtomicID::registerId(-1377,"sty")+
        AtomicID::registerId(-1378,"stz")+
        AtomicID::registerId(-1222,"meqrequest")+
        TypeInfoReg::addToRegistry(-1222,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1222,__construct_MeqRequest)+
        AtomicID::registerId(-1365,"meqcondeq")+
        TypeInfoReg::addToRegistry(-1365,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1365,__construct_MeqCondeq)+
        AtomicID::registerId(-1367,"meqsolver")+
        TypeInfoReg::addToRegistry(-1367,TypeInfo(TypeInfo::DYNAMIC,0))+
        DynamicTypeManager::addToRegistry(-1367,__construct_MeqSolver)+
    0;
    return res;
  }
  
  int __dum_call_registries_for_Meq = aidRegistry_Meq();

