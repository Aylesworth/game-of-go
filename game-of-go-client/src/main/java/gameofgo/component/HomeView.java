package gameofgo.component;

import javafx.scene.control.Tab;
import javafx.scene.control.TabPane;

public class HomeView extends TabPane {
    public HomeView() {
        this.setTabClosingPolicy(TabClosingPolicy.UNAVAILABLE);
        this.setTabDragPolicy(TabDragPolicy.FIXED);
        this.setTabMinWidth(100);
        this.getTabs().addAll(new PlayTab(), new Tab("Rankings"), new HistoryTab());
    }
}

