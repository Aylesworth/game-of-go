package gameofgo.component;

import gameofgo.common.Configs;
import javafx.scene.layout.BorderPane;

public class MainWindow extends BorderPane {
    private static MainWindow INSTANCE;

    public static MainWindow getInstance() {
        if (INSTANCE == null)
            INSTANCE = new MainWindow();
        return INSTANCE;
    }

    private MainWindow() {
        super();
        setPrefSize(Configs.WINDOW_WIDTH, Configs.WINDOW_HEIGHT);
    }
}
