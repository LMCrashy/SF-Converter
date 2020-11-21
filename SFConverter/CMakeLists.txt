FILE (GLOB 			SOURCE_FILES src/*.cpp src/SFLoader/*.cpp)
FILE (GLOB		 	HEADER_FILES include/*.h include/SFLoader/*.h)

include_directories(
	include
	include/SFLoader/
	include/external
	
	)

add_executable(SFConverter ${SOURCE_FILES} ${HEADER_FILES})