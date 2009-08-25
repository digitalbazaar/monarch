import dbcrypto
from dbcrypto import *

dbcrypto_init()

kf = KeyFactory()
privateKey = PrivateKey()
publicKey = PublicKey()
result = kf.createKeyPair("DSA", privateKey, publicKey)
print result
print kf.writePrivateKeyToPem(privateKey, "password")
print kf.writePublicKeyToPem(publicKey)
print privateKey.getAlgorithm()
print publicKey.getAlgorithm()

md5 = MessageDigest("MD5")
md5.update("THIS ")
md5.update("IS A")
md5.update(" MESSAGE")
digest = md5.getDigest()
print "calculated: " + digest
print "correct: 78eebfd9d42958e3f31244f116ab7bbe"

sha1 = MessageDigest("SHA1")
sha1.update("THIS IS A")
sha1.update(" MESSAGE")
digest = sha1.getDigest()
print "calculated: " + digest
print "correct: 5f24f4d6499fd2d44df6c6e94be8b14a796c071d"

ds = privateKey.createSignature()
ds.update(digest)
sig = ds.getValue()
print "length=%s" % len(sig)
b64 = sig.encode("base64")
print b64

ds = publicKey.createSignature()
ds.update(digest)
verified = ds.verify(sig)

print "verified=%s" % verified
keyFile = open("/work/src/dbcpp/dbcore/trunk/crypto/python/cppwrapper/storedkey.pem")
pem = keyFile.read();
loaded = kf.loadPrivateKeyFromPem(privateKey, pem, "password")
print "loaded: %s" % loaded

dbcrypto_cleanup()
