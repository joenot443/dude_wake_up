cmake_minimum_required(VERSION 3.21)
project(bpm_eval LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

get_filename_component(REPO "${CMAKE_CURRENT_SOURCE_DIR}/../.." ABSOLUTE)
set(GIST "${REPO}/src/Audio/gist")

add_executable(bpm_eval
    bpm_eval.cpp
    Trackers.cpp
    LegacyTracker.cpp
    NewTracker.cpp
    Taps.cpp
    # The detector the app used before BeatTracker (the baseline), and Gist/BTrack.
    legacy/BTrackDetector.cpp
    ${GIST}/BTrack.cpp ${GIST}/BTrackOnsetDetectionFunction.cpp
    ${GIST}/Gist.cpp ${GIST}/CoreTimeDomainFeatures.cpp ${GIST}/CoreFrequencyDomainFeatures.cpp
    ${GIST}/OnsetDetectionFunction.cpp ${GIST}/MFCC.cpp ${GIST}/Yin.cpp ${GIST}/WindowFunctions.cpp
    ${GIST}/kiss_fft.c ${GIST}/samplerate.c ${GIST}/src_sinc.c ${GIST}/src_linear.c ${GIST}/src_zoh.c)
target_include_directories(bpm_eval PRIVATE ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/legacy ${REPO}/src/Audio ${GIST} ${REPO}/src/Util)
target_compile_definitions(bpm_eval PRIVATE $<$<BOOL:${TRACE}>:BEATTRACKER_TRACE> USE_KISS_FFT _USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS NOMINMAX)
if(MSVC)
    target_compile_options(bpm_eval PRIVATE /O2 /utf-8 /W1)
endif()
