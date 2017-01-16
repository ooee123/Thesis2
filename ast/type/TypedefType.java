package ast.type;

import lombok.AllArgsConstructor;
import lombok.Data;

/**
 * Created by ooee on 9/24/16.
 */
@Data
@AllArgsConstructor
public class TypedefType implements ActualType {
    /**
     * The Original Type
     */
    private Type originalType;

    /**
     * The name of the typedef
     */
    private String typedefName;

    @Override
    public String toCode() {
        return typedefName;
    }

    @Override
    public String toExpandedCode() {
        return originalType.toExpandedCode();
    }
}
