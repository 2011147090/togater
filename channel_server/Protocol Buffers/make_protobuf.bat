set DST_DIR=C:\local
set SRC_DIR=C:\local
protoc -I=. --cpp_out=. packet.proto
