package gameofgo.common;

import java.util.HashMap;
import java.util.Map;

public class SessionStorage {
    private static Map<String, Object> items = new HashMap<>();

    private SessionStorage() {
    }

    public static void setItem(String key, Object item) {
        items.put(key, item);
    }

    public static Object getItem(String key) {
        return items.get(key);
    }

    public static void clear() {
        items.clear();
    }
}
