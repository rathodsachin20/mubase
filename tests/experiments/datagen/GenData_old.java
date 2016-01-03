import java.util.Random;

public class GenData {

    private static int N = 6; // Number of columns
    private static String[][] domains = {
{"Yellow", "Green", "Red", "Blue", "Black", "White", "Orange", "Purple", "Gray", "Silver", "Gold", "Maroon"},
{"Yellow", "Green", "Red", "Blue", "Black", "White", "Orange", "Purple", "Gray", "Silver", "Gold", "Maroon"},
{"Yellow", "Green", "Red", "Blue", "Black", "White", "Orange", "Purple", "Gray", "Silver", "Gold", "Maroon"},
{"Yellow", "Green", "Red", "Blue", "Black", "White", "Orange", "Purple", "Gray", "Silver", "Gold", "Maroon"},
{"Yellow", "Green", "Red", "Blue", "Black", "White", "Orange", "Purple", "Gray", "Silver", "Gold", "Maroon"},
{"Yellow", "Green", "Red", "Blue", "Black", "White", "Orange", "Purple", "Gray", "Silver", "Gold", "Maroon"}
                                        };
    public static void main(String[] args) throws Exception{
        Random   rseed = new Random(System.currentTimeMillis());
        Random[] rgen  = new Random[N];

        for (int i = 0; i < N; i++) {
            // Each generator to use a random seed
            Thread.sleep(rseed.nextInt(N));
            rgen[i] = new Random(System.currentTimeMillis());
        }

        int count = 1000; // generate 1000 records by default
        if (args.length > 0) {
            count = Integer.parseInt(args[0]);
        }

        StringBuffer sbuf = new StringBuffer();
        for (int row = 0; row  < count; row++) {
            for (int col = 0; col < N; col++) {
                int domIdx = rgen[col].nextInt(domains[col].length);
                sbuf.append(domains[col][domIdx]);
                if (col < N-1)
                    sbuf.append(",");
            }
            System.out.println(sbuf.toString());
            sbuf.setLength(0);
        }
    }
}
