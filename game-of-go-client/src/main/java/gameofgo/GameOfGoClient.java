package gameofgo;

import gameofgo.component.MainWindow;
import gameofgo.component.LoginView;
import gameofgo.service.SocketService;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class GameOfGoClient extends Application {
    @Override
    public void start(Stage stage) throws Exception {
        MainWindow mainWindow = MainWindow.getInstance();
        mainWindow.setCenter(new LoginView());
        stage.setScene(new Scene(mainWindow));
        stage.show();
    }

    @Override
    public void stop() throws Exception {
        SocketService.getInstance().close();
    }

    public static void main(String[] args) {
        launch(args);
    }
}
