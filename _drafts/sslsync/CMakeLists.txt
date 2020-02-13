ADD_EXECUTABLE(esyncclient sync_client.c)
TARGET_LINK_LIBRARIES(esyncclient -lssl -lcrypto)

ADD_EXECUTABLE(esyncserver sync_server.c)
TARGET_LINK_LIBRARIES(esyncserver -lssl -lcrypto)
