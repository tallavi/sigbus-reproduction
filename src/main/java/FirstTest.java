import org.junit.jupiter.api.Test;
import org.junit.platform.console.ConsoleLauncher;
import org.junit.platform.console.ConsoleLauncherExecutionResult;

class FirstTest  {

    public static void withoutJUnit(String[] args) throws InterruptedException {

        System.out.println("withoutJUnit START");

        System.out.println("withoutJUnit END");
    }

    public static void withJUnit(String[] args) throws InterruptedException {

        System.out.println("withJUnit START");
        ConsoleLauncherExecutionResult result = ConsoleLauncher.execute(System.out, System.err, args);
        System.out.println("withJUnit END");
    }

    @Test
    void myTest() throws InterruptedException {
        System.out.println("test START");
//         Thread.sleep(5000);
        System.out.println("test END");
    }
}
