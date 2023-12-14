package gameofgo.component;

import gameofgo.common.Configs;
import javafx.scene.Node;
import javafx.scene.layout.BorderPane;

import java.util.ArrayList;
import java.util.List;

public class MainWindow extends BorderPane {
    private static MainWindow INSTANCE;

    public static MainWindow getInstance() {
        if (INSTANCE == null)
            INSTANCE = new MainWindow();
        return INSTANCE;
    }

    private List<Node> views;

    private MainWindow() {
        super();
        setPrefSize(Configs.WINDOW_WIDTH, Configs.WINDOW_HEIGHT);
        views = new ArrayList<>();
    }

    public void next(Node node) {
        views.add(node);
        setCenter(node);
    }

    public void previous() {
        if (views.isEmpty()) return;

        views.remove(views.size() - 1);
        if (views.isEmpty()) setCenter(null);
        else setCenter(views.get(views.size() - 1));
    }

    public void swap(Node node) {
        if (views.isEmpty()) views.add(node);
        else views.set(views.size() - 1, node);
        setCenter(node);
    }
}
