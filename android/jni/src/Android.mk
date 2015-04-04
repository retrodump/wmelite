
LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := freetype
LOCAL_SRC_FILES := ../../../prebuilt/android/$(TARGET_ARCH_ABI)/libfreetype.a
include $(PREBUILT_STATIC_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := SDL2
LOCAL_SRC_FILES := ../../../prebuilt/android/$(TARGET_ARCH_ABI)/libSDL2.so

include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := bass
LOCAL_SRC_FILES := ../../../prebuilt/android/$(TARGET_ARCH_ABI)/libbass.so

include $(PREBUILT_SHARED_LIBRARY)
include $(CLEAR_VARS)

LOCAL_MODULE := freeimage
LOCAL_SRC_FILES := ../../../prebuilt/android/$(TARGET_ARCH_ABI)/libfreeimage.so

include $(PREBUILT_SHARED_LIBRARY)

include $(CLEAR_VARS)

LOCAL_MODULE := main

SDL_PATH := ../../../dependencies/SDL/jni/

LOCAL_C_INCLUDES := $(LOCAL_PATH)/$(SDL_PATH)/include \
$(LOCAL_PATH)/$(SDL_PATH)/src/core/android/ \
$(LOCAL_PATH)/$(SDL_PATH)/src/ \
$(LOCAL_PATH)/../../../dependencies/bass/ \
$(LOCAL_PATH)/../../../dependencies/libfreetype-android/jni/include/ \
$(LOCAL_PATH)/../../../dependencies/FreeImage3154/jni/Source/ \
$(LOCAL_PATH)/../../../dependencies/boost-1_53/build/include/boost-1_53/ \
$(LOCAL_PATH)/../../../src/

LOCAL_CPPFLAGS += -fexceptions -frtti -Wno-write-strings -Wno-conversion-null 

# LOCAL_ARM_MODE := arm

# Add your application source files here...
LOCAL_SRC_FILES := ../../../src/android/SDL_android_main.c \
../../../src/android/android.c \
../../../src/utils.cpp \
../../../src/BFileManager.cpp \
../../../src/PlatformSDL.cpp \
../../../src/StringUtil.cpp \
../../../src/SXString.cpp \
../../../src/ConvertUTF.c        \
../../../src/AdActor.cpp         \
../../../src/AdActorDir.cpp      \
../../../src/AdEntity.cpp \
../../../src/AdGame.cpp \
../../../src/AdInventoryBox.cpp \
../../../src/AdInventory.cpp \
../../../src/AdItem.cpp \
../../../src/AdLayer.cpp \
../../../src/AdNodeState.cpp \
../../../src/AdObject.cpp \
../../../src/AdPath.cpp \
../../../src/AdPathPoint.cpp \
../../../src/AdRegion.cpp \
../../../src/AdResponseBox.cpp \
../../../src/AdResponseContext.cpp \
../../../src/AdResponse.cpp \
../../../src/AdRotLevel.cpp \
../../../src/AdScaleLevel.cpp \
../../../src/AdScene.cpp \
../../../src/AdSceneNode.cpp \
../../../src/AdSceneState.cpp \
../../../src/AdSentence.cpp \
../../../src/AdSpriteSet.cpp \
../../../src/AdTalkDef.cpp \
../../../src/AdTalkHolder.cpp \
../../../src/AdTalkNode.cpp \
../../../src/AdWaypointGroup.cpp \
../../../src/BActiveRect.cpp \
../../../src/BBase.cpp \
../../../src/BDebugger.cpp \
../../../src/BDiskFile.cpp \
../../../src/BDynBuffer.cpp \
../../../src/BEvent.cpp \
../../../src/BFader.cpp  \
../../../src/BFile.cpp \
../../../src/BFileEntry.cpp \
../../../src/BFontBitmap.cpp \
../../../src/BFont.cpp \
../../../src/BFontStorage.cpp  \
../../../src/BFontTT.cpp \
../../../src/BFrame.cpp \
../../../src/BGame.cpp \
../../../src/BImage.cpp \
../../../src/BKeyboardState.cpp \
../../../src/BNamedObject.cpp \
../../../src/BObject.cpp \
../../../src/BPackage.cpp \
../../../src/BParser.cpp \
../../../src/BPersistMgr.cpp \
../../../src/BPkgFile.cpp \
../../../src/BPoint.cpp \
../../../src/BQuickMsg.cpp \
../../../src/BRegion.cpp \
../../../src/BRegistry.cpp \
../../../src/BRenderer.cpp \
../../../src/BRenderSDL.cpp \
../../../src/BResourceFile.cpp \
../../../src/BResources.cpp \
../../../src/BSaveThumbFile.cpp \
../../../src/BSaveThumbHelper.cpp \
../../../src/BScriptable.cpp \
../../../src/BScriptHolder.cpp \
../../../src/BSoundBuffer.cpp \
../../../src/BSound.cpp \
../../../src/BSoundMgr.cpp \
../../../src/BSprite.cpp \
../../../src/BStringTable.cpp \
../../../src/BSubFrame.cpp \
../../../src/BSurface.cpp \
../../../src/BSurfaceSDL.cpp \
../../../src/BSurfaceStorage.cpp \
../../../src/BTransitionMgr.cpp \
../../../src/BViewport.cpp \
../../../src/crc.cpp \
../../../src/DirectoryOperations.c \
../../../src/FileOperations.c \
../../../src/FontGlyphCache.cpp \
../../../src/main.cpp \
../../../src/MathUtil.cpp \
../../../src/Matrix4.cpp \
../../../src/PartEmitter.cpp \
../../../src/PartForce.cpp \
../../../src/PartParticle.cpp \
../../../src/PathUtil.cpp \
../../../src/ScEngine.cpp \
../../../src/ScScript.cpp \
../../../src/ScStack.cpp \
../../../src/ScValue.cpp \
../../../src/SdlUtil.cpp \
../../../src/SXArray.cpp \
../../../src/SXDate.cpp \
../../../src/SXFile.cpp \
../../../src/SXMath.cpp \
../../../src/SXMemBuffer.cpp \
../../../src/SxObject.cpp \
../../../src/SXStore.cpp \
../../../src/SysClass.cpp \
../../../src/SysClassRegistry.cpp \
../../../src/SysInstance.cpp \
../../../src/tinyxml.cpp \
../../../src/tinyxmlerror.cpp \
../../../src/tinyxmlparser.cpp \
../../../src/UIButton.cpp \
../../../src/UIEdit.cpp \
../../../src/UIEntity.cpp \
../../../src/UIObject.cpp \
../../../src/UIText.cpp \
../../../src/UITiledImage.cpp \
../../../src/UIWindow.cpp \
../../../src/Vector2.cpp \
#../../../src/VidDataSource.cpp \
#../../../src/VidManager.cpp \
#../../../src/VidTheoraPlayer.cpp \


LOCAL_SHARED_LIBRARIES := SDL2 freeimage bass
LOCAL_STATIC_LIBRARIES := freetype
LOCAL_LDLIBS := -lGLESv1_CM -llog -landroid -lz

include $(BUILD_SHARED_LIBRARY)

