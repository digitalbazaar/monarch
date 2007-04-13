/*
 * Copyright (c) 2006 Digital Bazar, Inc.  All rights reserved.
 */
import java.io.ByteArrayInputStream;
import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileOutputStream;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.GZIPInputStream;
import java.util.zip.GZIPOutputStream;
import java.util.zip.InflaterInputStream;

import com.db.stream.DeflaterInputStream;
import com.db.stream.GzipInputStream;
import com.db.stream.InflaterOutputStream;
import com.db.stream.UnGzipOutputStream;

/**
 * This class is used to test GZIP capability.
 *
 * @author Dave Longley
 */
public class UTGzip
{
   /**
    * A lot of WSDL content to gzip and ungzip for testing.
    */
   public static final String wsdlContent =
      "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\r\n" + 
      "<definitions xmlns=\"http://schemas.xmlsoap.org/wsdl/\" xmlns:tns=\"urn:BitmunkSVA\" xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" xmlns:soap=\"http://schemas.xmlsoap.org/wsdl/soap/\" name=\"BitmunkSVA\" targetNamespace=\"urn:BitmunkSVA\">\r\n" + 
      " <types/>\r\n" + 
      " <message name=\"IBitmunkSVA_soapBFPVerifySellerFile\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapBFPVerifySellerFileResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapBFPVerifyBuyerFile\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapBFPVerifyBuyerFileResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapBFPVerifySellerSoftware\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapBFPVerifySellerSoftwareResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetContract\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetContractResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapUpdateContract\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapUpdateContractResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapAuthorizeContract\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapAuthorizeContractResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapIsContractAuthorized\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapIsContractAuthorizedResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetDecryptMethod\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetDecryptMethodResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapVoidContract\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapVoidContractResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapAuthorizeWebBuyContract\">\r\n" + 
      "  <part name=\"long_1\" type=\"xsd:long\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapAuthorizeWebBuyContractResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapIsContractWebBuyAuthorized\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapIsContractWebBuyAuthorizedResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapCompleteWebBuyContract\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapCompleteWebBuyContractResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapProcessSellerReview\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapProcessSellerReviewResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetFinancialAccounts\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetFinancialAccountsResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapUpdateSellerCatalog\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapUpdateSellerCatalogResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapUpdateCreatorCatalog\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapUpdateCreatorCatalogResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetCatalogPayees\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetCatalogPayeesResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetMediaInfo\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetMediaInfoResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGenerateDWUploadAuthKey\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGenerateDWUploadAuthKeyResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapSearch\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapSearchResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetSellerList\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetSellerListResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapFileSupportRequest\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapFileSupportRequestResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetPublicKeyForUserId\">\r\n" + 
      "\r\n" + 
      "  <part name=\"long_1\" type=\"xsd:long\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetPublicKeyForUserIdResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetPublicKeyForUsername\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetPublicKeyForUsernameResponse\">\r\n" + 
      "\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapSetPublicKeyForUserId\">\r\n" + 
      "  <part name=\"long_1\" type=\"xsd:long\"/>\r\n" + 
      "  <part name=\"String_2\" type=\"xsd:string\"/>\r\n" + 
      "  <part name=\"String_3\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapSetPublicKeyForUserIdResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      "\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapSetPublicKeyForUsername\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      "  <part name=\"String_2\" type=\"xsd:string\"/>\r\n" + 
      "  <part name=\"String_3\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapSetPublicKeyForUsernameResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      "\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetUserId\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetUserIdResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:long\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapVerifyPassword\">\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      "  <part name=\"String_2\" type=\"xsd:string\"/>\r\n" + 
      "\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapVerifyPasswordResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:boolean\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetSupportedClientVersions\"/>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetSupportedClientVersionsResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetClientUpdateScript\">\r\n" + 
      "\r\n" + 
      "  <part name=\"String_1\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapGetClientUpdateScriptResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      " <message name=\"IBitmunkSVA_soapPing\"/>\r\n" + 
      " <message name=\"IBitmunkSVA_soapPingResponse\">\r\n" + 
      "  <part name=\"result\" type=\"xsd:string\"/>\r\n" + 
      " </message>\r\n" + 
      "\r\n" + 
      " <portType name=\"IBitmunkSVA\">\r\n" + 
      "  <operation name=\"soapBFPVerifySellerFile\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapBFPVerifySellerFile\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapBFPVerifySellerFileResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapBFPVerifyBuyerFile\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapBFPVerifyBuyerFile\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapBFPVerifyBuyerFileResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "\r\n" + 
      "  <operation name=\"soapBFPVerifySellerSoftware\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapBFPVerifySellerSoftware\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapBFPVerifySellerSoftwareResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetContract\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetContract\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetContractResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapUpdateContract\" parameterOrder=\"String_1\">\r\n" + 
      "\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapUpdateContract\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapUpdateContractResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapAuthorizeContract\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapAuthorizeContract\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapAuthorizeContractResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapIsContractAuthorized\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapIsContractAuthorized\"/>\r\n" + 
      "\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapIsContractAuthorizedResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetDecryptMethod\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetDecryptMethod\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetDecryptMethodResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapVoidContract\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapVoidContract\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapVoidContractResponse\"/>\r\n" + 
      "\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapAuthorizeWebBuyContract\" parameterOrder=\"long_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapAuthorizeWebBuyContract\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapAuthorizeWebBuyContractResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapIsContractWebBuyAuthorized\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapIsContractWebBuyAuthorized\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapIsContractWebBuyAuthorizedResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "\r\n" + 
      "  <operation name=\"soapCompleteWebBuyContract\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapCompleteWebBuyContract\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapCompleteWebBuyContractResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapProcessSellerReview\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapProcessSellerReview\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapProcessSellerReviewResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetFinancialAccounts\" parameterOrder=\"String_1\">\r\n" + 
      "\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetFinancialAccounts\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetFinancialAccountsResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapUpdateSellerCatalog\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapUpdateSellerCatalog\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapUpdateSellerCatalogResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapUpdateCreatorCatalog\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapUpdateCreatorCatalog\"/>\r\n" + 
      "\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapUpdateCreatorCatalogResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetCatalogPayees\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetCatalogPayees\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetCatalogPayeesResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetMediaInfo\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetMediaInfo\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetMediaInfoResponse\"/>\r\n" + 
      "\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGenerateDWUploadAuthKey\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGenerateDWUploadAuthKey\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGenerateDWUploadAuthKeyResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapSearch\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapSearch\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapSearchResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "\r\n" + 
      "  <operation name=\"soapGetSellerList\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetSellerList\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetSellerListResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapFileSupportRequest\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapFileSupportRequest\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapFileSupportRequestResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetPublicKeyForUserId\" parameterOrder=\"long_1\">\r\n" + 
      "\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetPublicKeyForUserId\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetPublicKeyForUserIdResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetPublicKeyForUsername\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetPublicKeyForUsername\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetPublicKeyForUsernameResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapSetPublicKeyForUserId\" parameterOrder=\"long_1,String_2,String_3\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapSetPublicKeyForUserId\"/>\r\n" + 
      "\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapSetPublicKeyForUserIdResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapSetPublicKeyForUsername\" parameterOrder=\"String_1,String_2,String_3\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapSetPublicKeyForUsername\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapSetPublicKeyForUsernameResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetUserId\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetUserId\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetUserIdResponse\"/>\r\n" + 
      "\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapVerifyPassword\" parameterOrder=\"String_1,String_2\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapVerifyPassword\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapVerifyPasswordResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetSupportedClientVersions\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetSupportedClientVersions\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetSupportedClientVersionsResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "\r\n" + 
      "  <operation name=\"soapGetClientUpdateScript\" parameterOrder=\"String_1\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapGetClientUpdateScript\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapGetClientUpdateScriptResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapPing\">\r\n" + 
      "   <input message=\"tns:IBitmunkSVA_soapPing\"/>\r\n" + 
      "   <output message=\"tns:IBitmunkSVA_soapPingResponse\"/>\r\n" + 
      "  </operation>\r\n" + 
      " </portType>\r\n" + 
      "\r\n" + 
      " <binding name=\"IBitmunkSVABinding\" type=\"tns:IBitmunkSVA\">\r\n" + 
      " <soap:binding transport=\"http://schemas.xmlsoap.org/soap/http\" style=\"rpc\"/>\r\n" + 
      "  <operation name=\"soapBFPVerifySellerFile\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapBFPVerifyBuyerFile\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapBFPVerifySellerSoftware\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetContract\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapUpdateContract\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapAuthorizeContract\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapIsContractAuthorized\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetDecryptMethod\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapVoidContract\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapAuthorizeWebBuyContract\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapIsContractWebBuyAuthorized\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapCompleteWebBuyContract\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapProcessSellerReview\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetFinancialAccounts\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapUpdateSellerCatalog\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapUpdateCreatorCatalog\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetCatalogPayees\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetMediaInfo\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGenerateDWUploadAuthKey\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapSearch\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetSellerList\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapFileSupportRequest\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetPublicKeyForUserId\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetPublicKeyForUsername\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapSetPublicKeyForUserId\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapSetPublicKeyForUsername\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetUserId\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapVerifyPassword\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetSupportedClientVersions\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapGetClientUpdateScript\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      "  <operation name=\"soapPing\">\r\n" + 
      "   <soap:operation soapAction=\"\"/>\r\n" + 
      "   <input>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "   </input>\r\n" + 
      "   <output>\r\n" + 
      "    <soap:body encodingStyle=\"http://schemas.xmlsoap.org/soap/encoding/\" use=\"encoded\" namespace=\"urn:BitmunkSVA\"/>\r\n" + 
      "\r\n" + 
      "   </output>\r\n" + 
      "  </operation>\r\n" + 
      " </binding>\r\n" + 
      " <service name=\"BitmunkSVA\">\r\n" + 
      "  <port name=\"IBitmunkSVAPort\" binding=\"tns:IBitmunkSVABinding\">\r\n" + 
      "   <soap:address location=\"https://localhost:8176/sva/\" xmlns=\"http://schemas.xmlsoap.org/wsdl/\"/>\r\n" + 
      "  </port>\r\n" + 
      " </service>\r\n" + 
      "</definitions>";
   
   /**
    * A deflate test using Java's built in deflate and inflate classes.
    */
   public static void builtInDeflateTest()
   {
      System.out.println("Built-in Java Deflate test:");
      
      try
      {
         // the content to deflate
         String content = wsdlContent;
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // deflater
         DeflaterOutputStream dos = new DeflaterOutputStream(fos);
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = bais.read(b)) != -1)
         {
            // write content out
            dos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         dos.close();
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // inflater
         InflaterInputStream iis = new InflaterInputStream(fis);
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = iis.read(b)) != -1)
         {
            // write content out
            baos.write(b, 0, numBytes);
         }
         
         // close inflater
         iis.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A deflate test using Java's built in deflate class and com.db.stream's
    * inflate class.
    */
   public static void javaDeflaterDBInflaterTest()
   {
      System.out.println("Java Deflate DB Inflate test:");
      
      try
      {
         // the content to deflate
         String content = wsdlContent;
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // deflater
         DeflaterOutputStream dos = new DeflaterOutputStream(fos);
         
         // read until content is zipped
         long totalRead = 0;
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = bais.read(b)) != -1)
         {
            totalRead += numBytes;
            
            // write content out
            dos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         dos.close();
         
         System.out.println("total read=" + totalRead);
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // inflater
         InflaterOutputStream ios = new InflaterOutputStream(baos);
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ios.write(b, 0, numBytes);
         }
         
         // close streams
         fis.close();
         ios.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A deflate test using DB's deflater and Java's built in inflater.
    */
   public static void dBDeflaterJavaInflaterTest()
   {
      System.out.println("DB Deflate Java Inflate test:");
      
      try
      {
         // the content to deflate
         String content = wsdlContent;
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // deflater
         DeflaterInputStream dis = new DeflaterInputStream(bais);
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = dis.read(b)) != -1)
         {
            // write content out
            fos.write(b, 0, numBytes);
         }
         
         // close streams
         dis.close();
         fos.close();
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // inflater
         InflaterInputStream iis = new InflaterInputStream(fis);
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = iis.read(b)) != -1)
         {
            // write content out
            baos.write(b, 0, numBytes);
         }
         
         // close inflater
         iis.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }  
   
   /**
    * A deflate test using all com.db.stream code.
    */
   public static void dBDeflateTest()
   {
      System.out.println("DB Deflate test:");
      
      try
      {
         // the content to deflate
         String content = wsdlContent;
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // deflater
         DeflaterInputStream dis = new DeflaterInputStream(bais);
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // read until content is zipped
         long totalRead = 0;
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = dis.read(b)) != -1)
         {
            totalRead += numBytes;
            
            // write content out
            fos.write(b, 0, numBytes);
         }
         
         // close streams
         dis.close();
         fos.close();
         
         System.out.println("total read=" + totalRead);
         
         // read deflated content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // inflater
         InflaterOutputStream ios = new InflaterOutputStream(baos);
         
         // read until content is inflated
         numBytes = -1;
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ios.write(b, 0, numBytes);
         }
         
         // close streams
         fis.close();
         ios.close();
         
         // get content
         String inflatedContent = baos.toString();
         
         System.out.println("Inflated content='" + inflatedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }   
   
   /**
    * A gzip test using Java's built in GZIP classes.
    */
   public static void builtInGZIPTest()
   {
      System.out.println("Built-in Java GZIP test:");
      
      try
      {
         // the content to gzip
         String content = wsdlContent;
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // gzipper
         GZIPOutputStream gzos = new GZIPOutputStream(fos);
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = bais.read(b)) != -1)
         {
            // write content out
            gzos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         gzos.close();
         
         System.out.println("total written=" + new File("test.zip").length());
         
         // read zipped content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // ungzipper
         GZIPInputStream gzis = new GZIPInputStream(fis);
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // read until content is unzipped
         numBytes = -1;
         while((numBytes = gzis.read(b)) != -1)
         {
            // write content out
            baos.write(b, 0, numBytes);
         }
         
         // close ungzipper
         gzis.close();
         
         // get content
         String unzippedContent = baos.toString();
         
         System.out.println("Unzipped content='" + unzippedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test using Java's built-in gzipper and com.db.stream's ungzipper.
    */
   public static void javaGZipperDBUnGZipperTest()
   {
      System.out.println("Java GZIP, DB UN-GZIP Test:");
      
      try
      {
         // the content to gzip
         String content = wsdlContent;
         System.out.println("test content length=" + content.length());
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // gzipper
         GZIPOutputStream gzos = new GZIPOutputStream(fos);
         
         // read until content is zipped
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = bais.read(b)) != -1)
         {
            // write content out
            gzos.write(b, 0, numBytes);
         }
         
         // close streams
         bais.close();
         gzos.close();
         
         System.out.println("total written=" + new File("test.zip").length()); 
         
         // read zipped content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();

         // ungzipper
         UnGzipOutputStream ungzos = new UnGzipOutputStream(baos); 
         
         // read until content is unzipped
         numBytes = -1;
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ungzos.write(b, 0, numBytes);
         }
         
         // close file input stream
         fis.close();
         
         // close ungzipper
         ungzos.close();
         
         // get content
         String unzippedContent = baos.toString();
         
         System.out.println("Unzipped content='" + unzippedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test using com.db.stream's gzipper and Java's built-in ungzipper.
    */
   public static void dBGZipperjavaUnGZipperTest()
   {
      System.out.println("DB GZIP, Java UN-GZIP Test:");
      
      try
      {
         // the content to gzip
         String content = wsdlContent;
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // gzipper
         GzipInputStream gzis = new GzipInputStream(bais);

         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // read until content is zipped
         long totalWritten = 0;
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = gzis.read(b)) != -1)
         {
            // write content out
            fos.write(b, 0, numBytes);
            totalWritten += numBytes;
         }
         
         // close streams
         gzis.close();
         fos.close();
         
         System.out.println("total written=" + totalWritten);
         
         // read zipped content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // ungzipper
         UnGzipOutputStream ungzos = new UnGzipOutputStream(baos); 
         
         // read until content is unzipped
         numBytes = -1;
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ungzos.write(b, 0, numBytes);
         }
         
         // close file input stream
         fis.close();
         
         // close ungzipper
         ungzos.close();         
         
         // get content
         String unzippedContent = baos.toString();
         
         System.out.println("Unzipped content='" + unzippedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }
   
   /**
    * A test using all com.db.stream gzip code.
    */
   public static void dBGZipTest()
   {
      System.out.println("DB GZIP Test:");
      
      try
      {
         // the content to gzip
         String content = wsdlContent;
         
         // input stream to read the content with
         ByteArrayInputStream bais = new ByteArrayInputStream(
            content.getBytes());
         
         // gzipper
         GzipInputStream gzis = new GzipInputStream(bais);

         // file to gzip to
         FileOutputStream fos = new FileOutputStream("test.zip");

         // read until content is zipped
         long totalWritten = 0;
         int numBytes = -1;
         byte[] b = new byte[65536];
         while((numBytes = gzis.read(b)) != -1)
         {
            // write content out
            fos.write(b, 0, numBytes);
            totalWritten += numBytes;
         }
         
         // close streams
         gzis.close();
         fos.close();
         
         System.out.println("total written=" + totalWritten);
         
         // read zipped content
         FileInputStream fis = new FileInputStream("test.zip");
         
         // output stream to write content with
         ByteArrayOutputStream baos = new ByteArrayOutputStream();
         
         // ungzipper
         UnGzipOutputStream ungzos = new UnGzipOutputStream(baos); 
         
         // read until content is unzipped
         numBytes = -1;
         while((numBytes = fis.read(b)) != -1)
         {
            // write content out
            ungzos.write(b, 0, numBytes);
         }
         
         // close file input stream
         fis.close();
         
         // close ungzipper
         ungzos.close();         
         
         // get content
         String unzippedContent = baos.toString();
         
         System.out.println("Unzipped content='" + unzippedContent + "'");
      }
      catch(Throwable t)
      {
         t.printStackTrace();
      }
   }   
   
   /**
    * The main method that runs the gzip test.
    * 
    * @param args the args for the main method.
    */
   public static void main(String[] args)
   {
      // run the built-in deflate test
      //builtInDeflateTest();
      
      //System.out.println();
      
      // run the java deflater, db inflater test
      //javaDeflaterDBInflaterTest();
      
      //System.out.println();
      
      // run the db deflater, java inflater test
      //dBDeflaterJavaInflaterTest();
      
      //System.out.println();
      
      // run all db deflation/inflation code
      //dBDeflateTest();
      
      //System.out.println();
      
      // run the built-in gzip test
      //builtInGZIPTest();
      
      //System.out.println();
      
      // run the java gzipper, db ungzipper test
      //javaGZipperDBUnGZipperTest();
      
      //System.out.println();
      
      // run the db gzipper, java ungzipper test
      //dBGZipperjavaUnGZipperTest();
      
      //System.out.println();
      
      // run the db gzipper code
      dBGZipTest();
   }
}
