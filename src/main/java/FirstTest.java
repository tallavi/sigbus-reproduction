import org.junit.jupiter.api.Test;

class FirstTest  {

    public static void main(String[] args) throws InterruptedException {
        System.out.println("myMainA START");
        Thread.sleep(5000);
        System.out.println("myMain END");
    }

    @Test
    void myTest() throws InterruptedException {
        System.out.println("test START");
        Thread.sleep(5000);
        System.out.println("test END");
    }
}
