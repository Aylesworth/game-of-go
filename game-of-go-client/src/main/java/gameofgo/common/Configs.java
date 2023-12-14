package gameofgo.common;

import javafx.scene.paint.Color;
import javafx.scene.text.Font;
import javafx.scene.text.FontWeight;

public class Configs {
    public static final double WINDOW_WIDTH = 1280;
    public static final double WINDOW_HEIGHT = 900;
    public static final Color ERROR_COLOR = Color.PINK;

    public static Font primaryFont(double size) {
        return primaryFont(FontWeight.NORMAL, size);
    }

    public static Font primaryFont(FontWeight fontWeight, double size) {
        return Font.font("Avant Garde", fontWeight, size);
    }
}
