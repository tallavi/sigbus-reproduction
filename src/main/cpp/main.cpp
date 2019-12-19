#include <iostream>
#include <jni.h>
#include <unistd.h>
#include <string>
#include <boost/filesystem.hpp>
#include <sstream>
#include <signal.h>
#include <setjmp.h>


jmp_buf ljenv;

std::string V2X_FILE_NAME = "jars/sigbus-reproduction.jar";

#define MAIN_CLASS "org/junit/platform/console/ConsoleLauncher"
//#define MAIN_CLASS "FirstTest"

//#define HANDLE_SIGNALS

// OS X uses SIGBUS in case of accessing incorrect memory region
// Linux will use SIGSEGV - this is why we should use two handlers
// there are 31 possible signals we can handle
struct sigaction actions[31];

// this function will set the handler for a signal
void setup_signal_handler(int sig, void (*handler)( int ), struct sigaction *old) {
    struct sigaction action;

    // fill action structure
    // pointer to function that will handle signal
    action.sa_handler = handler;

    // for the masks description take a look
    // at "man sigaction"
    sigemptyset(&(action.sa_mask));
    sigaddset(&(action.sa_mask), sig);

    // you can bring back original signal using
    // SA_RESETHAND passed inside sa_flags
    action.sa_flags = 0;

    // and set new handler for signal
    sigaction(sig, &action, &actions[sig - 1]);
}

// this function will be called whenever signal occurs
void handler(int handle) {
    // be very condense here
    // just do essential stuff and get
    // back to the place you want to be
    write(STDOUT_FILENO, "Hello from handler\n", strlen("Hello from handler\n"));
    // set original signal handler
    sigaction(handle, &actions[handle - 1], NULL);
    // and jump to where we have set long jump
    siglongjmp(ljenv, 1);
}

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

#ifdef HANDLE_SIGNALS
    // setup signal handlers
    // signals are counted from 1 - 31. Array indexes are
    // counted from 0 - 30. This is why we do 10-1 and 11-1
    setup_signal_handler(10, handler, &actions[10 - 1]);
    setup_signal_handler(11, handler, &actions[11 - 1]);
#endif

    jmethodID mainMethod = env->GetStaticMethodID(mainClass, "main", "([Ljava/lang/String;)V");

    if( sigsetjmp(ljenv,1) == 0) {
//         call the code that will fail

        std::cout << "Before call" << std::endl;
        env->CallStaticVoidMethod(mainClass, mainMethod, argsArray);
        std::cout << "After call" << std::endl;

    } else {
        std::cout << "handled a signal" << std::endl;
    }

#ifdef HANDLE_SIGNALS
    // if everything was OK, we can set old handlers
    sigaction(10, &actions[10 - 1], NULL);
    sigaction(11, &actions[11 - 1], NULL);
#endif


    jvm->DestroyJavaVM();

    std::cout << "CppMainEnd" << std::endl;
    return 0;
}