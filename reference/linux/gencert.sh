#!/bin/bash

mkdir pki/{CA,CLI,SVR} -p

openssl genrsa -out pki/CA/cakey.pem 2048
openssl req -new -key pki/CA/cakey.pem -out pki/CA/ca.csr -subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyCA"
openssl x509 -req -days 3650 -sha1 -extensions v3_ca -signkey pki/CA/cakey.pem -in pki/CA/ca.csr -out pki/CA/cacert.pem

openssl genrsa -out pki/SVR/key.pem 2048
openssl req -new -key pki/SVR/key.pem -out pki/SVR/server.csr -subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyServer"
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA pki/CA/cacert.pem -CAkey pki/CA/cakey.pem -CAserial pki/CA/ca.srl -CAcreateserial -in pki/SVR/server.csr -out pki/SVR/cert.pem
openssl verify -CAfile pki/CA/cacert.pem pki/SVR/cert.pem

openssl genrsa -out pki/CLI/key.pem 2048
openssl req -new -key pki/CLI/key.pem -out pki/CLI/client.csr -subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyClient"
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA pki/CA/cacert.pem -CAkey pki/CA/cakey.pem -CAserial pki/CA/ca.srl -in pki/CLI/client.csr -out pki/CLI/cert.pem
openssl verify -CAfile pki/CA/cacert.pem pki/CLI/cert.pem

openssl genrsa -aes256 -out pki/CLI/keysec.pem -passout pass:123456 2048
openssl req -new -key pki/CLI/keysec.pem -out pki/CLI/clientsec.csr -subj "/C=CN/ST=MyProvince/L=MyCity/O=MyOrganization/OU=MyGroup/CN=MyClient" -passin pass:123456
openssl x509 -req -days 365 -sha1 -extensions v3_req -CA pki/CA/cacert.pem -CAkey pki/CA/cakey.pem -CAserial pki/CA/ca.srl -in pki/CLI/clientsec.csr -out pki/CLI/certsec.pem -passin pass:123456
openssl rsa -in pki/CLI/keysec.pem -out pki/CLI/keyplain.pem -passin pass:123456

#openssl s_server -accept 8090 -CAfile pki/CA/cacert.pem -key pki/SVR/key.pem -cert pki/SVR/cert.pem -state
#openssl s_client -connect 127.0.0.1:8090 -CAfile pki/CA/cacert.pem -key pki/CLI/key.pem -cert pki/CLI/cert.pem -state
