package gameofgo.component;

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
        setPrefSize(1280, 900);
    }
}
