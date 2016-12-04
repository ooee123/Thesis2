package ast.type;

import lombok.NonNull;
import lombok.Value;

/**
 * Created by ooee on 9/27/16.
 */
@Value
public class PointerType implements Type {
    private int pointerLevel;
    @NonNull private ActualType actualType;

    @Override
    public String toCode() {
        StringBuilder builder = new StringBuilder();
        builder.append(actualType.toCode() + " ");

        for (int i = 0; i < pointerLevel; i++) {
            builder.append("*");
        }
        return builder.toString();
    }
}
