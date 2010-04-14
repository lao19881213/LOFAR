/* DO NOT EDIT THIS FILE - it is machine generated */
#include <jni.h>
/* Header for class nl_astron_lofar_sas_otb_jotdb3_jCampaign */

#ifndef _Included_nl_astron_lofar_sas_otb_jotdb3_jCampaign
#define _Included_nl_astron_lofar_sas_otb_jotdb3_jCampaign
#ifdef __cplusplus
extern "C" {
#endif
/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    initCampaign
 * Signature: ()V
 */
JNIEXPORT void JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_initCampaign
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    getCampaign
 * Signature: (Ljava/lang/String;)Lnl/astron/lofar/sas/otb/jotdb3/jCampaignInfo;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_getCampaign__Ljava_lang_String_2
  (JNIEnv *, jobject, jstring);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    getCampaign
 * Signature: (I)Lnl/astron/lofar/sas/otb/jotdb3/jCampaignInfo;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_getCampaign__I
  (JNIEnv *, jobject, jint);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    getCampaignList
 * Signature: ()Ljava/util/Vector;
 */
JNIEXPORT jobject JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_getCampaignList
  (JNIEnv *, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    saveCampaign
 * Signature: (Lnl/astron/lofar/sas/otb/jotdb3/jCampaignInfo;)I
 */
JNIEXPORT jint JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_saveCampaign
  (JNIEnv *, jobject, jobject);

/*
 * Class:     nl_astron_lofar_sas_otb_jotdb3_jCampaign
 * Method:    errorMsg
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_nl_astron_lofar_sas_otb_jotdb3_jCampaign_errorMsg
  (JNIEnv *, jobject);

#ifdef __cplusplus
}
#endif
#endif
