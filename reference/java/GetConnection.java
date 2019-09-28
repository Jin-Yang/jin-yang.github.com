// javac GetConnection.java
// java -classpath .:/usr/share/java/mysql-connector-java.jar GetConnection

import java.sql.*;
import java.text.SimpleDateFormat;

public class GetConnection {
    public static void main(String[] args){
        try{
            // 调用Class.forName()方法加载驱动程序
            Class.forName("com.mysql.jdbc.Driver");
            System.out.println("成功加载MySQL驱动！");
        }catch(ClassNotFoundException e1){
            System.out.println("找不到MySQL驱动!");
            e1.printStackTrace();
        }

        String url="jdbc:mysql://localhost:3306/mysql";    // JDBC的URL
        // 调用DriverManager对象的getConnection()方法，获得一个Connection对象
        try {
            Connection conn;
            conn = DriverManager.getConnection(url,"root","password");
            Statement stmt = conn.createStatement(); // 创建Statement对象
            System.out.println("成功连接到数据库！");
            ResultSet rs = stmt.executeQuery("select now() now");
            while(rs.next()){
                //Retrieve by column name
                java.sql.Timestamp time = rs.getTimestamp("now");
                SimpleDateFormat sdf = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
                String str = sdf.format(time);
                System.out.println("NOW: " + str);
            }
            System.out.println("释放相关资源！");
            rs.close();
            stmt.close();
            conn.close();
        } catch (SQLException e){
            e.printStackTrace();
        }
    }
}
