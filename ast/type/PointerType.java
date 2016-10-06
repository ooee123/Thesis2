package ast.type;

import lombok.Value;

/**
 * Created by ooee on 9/27/16.
 */
@Value
public class PointerType implements Type {
    private int pointerLevel;
    private ActualType actualType;

    @Override
    public String toCode() {
        StringBuilder builder = new StringBuilder();
        for (int i = 0; i < pointerLevel; i++) {
            builder.append("*");
        }
        builder.append(actualType.toCode());
        return builder.toString();
    }
}
