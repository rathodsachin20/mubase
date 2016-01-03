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

{"Yellow", "Blue", "Red", "Green", "Black", "White", "Orange", "Purple", "Brown", "Gray", "Magenta", "Pink", "cyan", "Browser", "Silver", "Gold", "Grey", "Navy", "Turquoise", "Transparent", "Maroon", "Violet", "Tan", "Teal", "Ivory", "Royal", "Aqua", "Burgundy", "Olive", "Lime", "Chartreuse", "lightGray", "Beige", "Darkgray", "Fuchsia", "Indigo", "Lavender", "Clear", "Peach", "Flesh", "Toyota", "BMW", "Nissan", "Mazda", "Mitsubishi", "Honda", "Subaru", "Volvo", "Ford", "Volkswagen", "Suzuki", "Saab", "Porsche", "Lexus", "Jaguar", "Pontiac", "Jeep", "Audi", "Kia", "Isuzu", "Chevrolet", "Dodge", "Saturn", "Hyundai", "Oldsmobile", "Peugeot", "Chrysler", "Mercury", "Buick", "Lincoln", "Renault", "Plymouth", "Lotus", "Infiniti", "Ferrari", "Acura", "Cadillac", "Mercedes", "Lamborghini", "Fiat", "GMC", "Triumph", "MG", "Rover", "Maserati"}

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
