package gameofgo.common;

public record Message(
        String messageType,
        String payload
) {
}
