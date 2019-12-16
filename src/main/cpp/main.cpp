#include <iostream>
#include <jni.h>
#include <unistd.h>
#include <string>
#include <boost/filesystem.hpp>
#include <sstream>

std::string V2X_FILE_NAME = "jars/sigbus-reproduction.jar";

#define MAIN_CLASS "org/junit/platform/console/ConsoleLauncher"
//#define MAIN_CLASS "MyTest"

int main(int argc, char **argv) {

    std::cout << "CppMainStart" << std::endl;

    std::cout << "current_path: " << boost::filesystem::current_path().string() << std::endl;

    auto directIterator = boost::filesystem::directory_iterator("jars");
    auto directIteratorEnd = boost::filesystem::end(directIterator);
    boost::system::error_code ec;

    std::stringstream classPathSS;

    bool isFirstJar = true;

    for(; directIterator != directIteratorEnd; directIterator.increment(ec)) {

        auto jarPath = directIterator->path().string();

        std::cout << "Loading JAR: " << jarPath << std::endl;

        if (isFirstJar)
            classPathSS << "-Djava.class.path=";
        else
            classPathSS << ":";

        classPathSS << jarPath;

        isFirstJar = false;
    }

    JavaVM *jvm;       /* denotes a Java VM */
    JNIEnv *env;       /* pointer to native method interface */
    JavaVMInitArgs vm_args; /* JDK/JRE 6 VM initialization arguments */
    JavaVMOption* options = new JavaVMOption[1];

    std::string classPath(classPathSS.str());
    options[0].optionString = const_cast<char *>(classPath.c_str());
    vm_args.version = JNI_VERSION_1_6; //using 1.6 to match android
    vm_args.nOptions = 1;
    vm_args.options = options;
    vm_args.ignoreUnrecognized = false;
    /* load and initialize a Java VM, return a JNI interface
     * pointer in env */
    long status = JNI_CreateJavaVM(&jvm, (void**)&env, &vm_args);

    if (status == JNI_ERR){
        std::cout << "Fail: Unable to load JVM \t Exit" << std::endl;
    }
    else if (status == JNI_OK){
        std::cout << "CreateVM:\t\tJVM loaded successfully!" << std::endl ;
    }

    delete[] options;
    /* invoke the Main.test method using the JNI */
    jclass mainClass = env->FindClass(MAIN_CLASS);

    jthrowable exc;
    exc = env->ExceptionOccurred();
    if (exc) {
        jclass newExcCls;
        env->ExceptionDescribe();
        env->ExceptionClear();
    }

    jclass stringClass = env->FindClass("java/lang/String");

    jobject emptyStringObject = env->NewStringUTF("");

    jobjectArray argsArray = env->NewObjectArray(3, stringClass, emptyStringObject);

    env->SetObjectArrayElement(argsArray, 0, env->NewStringUTF("--class-path"));
    env->SetObjectArrayElement(argsArray, 1, env->NewStringUTF(V2X_FILE_NAME.c_str()));
    env->SetObjectArrayElement(argsArray, 2, env->NewStringUTF((std::string("--scan-classpath")).c_str()));

    jmethodID mainMethod = env->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");
    env->CallStaticVoidMethod(mainClass, mainMethod, argsArray);

    jvm->DestroyJavaVM();

    std::cout << "CppMainEnd" << std::endl;
    return 0;
}