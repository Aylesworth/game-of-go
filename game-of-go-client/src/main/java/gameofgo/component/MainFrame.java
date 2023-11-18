package gameofgo.component;

import javafx.scene.layout.BorderPane;

public class MainFrame extends BorderPane {
    private static MainFrame INSTANCE;

    public static MainFrame getInstance() {
        if (INSTANCE == null)
            INSTANCE = new MainFrame();
        return INSTANCE;
    }
    private MainFrame() {
        super();
        setPrefSize(1280, 720);
    }
}
