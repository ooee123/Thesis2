package ast.type;

import lombok.NonNull;
import lombok.Value;

/**
 * Created by ooee on 9/25/16.
 */
@Value
public class PrimitiveType implements ActualType {
    @NonNull private String typeName;

    @Override
    public String toCode() {
        return typeName;
    }

    @Override
    public String toExpandedCode() {
        return toCode();
    }
}
