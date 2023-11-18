package gameofgo;

import gameofgo.component.GamePanel;
import gameofgo.component.MainFrame;
import gameofgo.component.SignInPanel;
import gameofgo.service.SocketService;
import javafx.application.Application;
import javafx.scene.Scene;
import javafx.stage.Stage;

public class GameOfGoClient extends Application {
    @Override
    public void start(Stage stage) throws Exception {
        MainFrame mainFrame = MainFrame.getInstance();
        mainFrame.setCenter(new SignInPanel());
        stage.setScene(new Scene(mainFrame));
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
