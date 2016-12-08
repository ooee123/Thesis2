package ast.type;

import lombok.Value;

import java.util.List;
import java.util.stream.Collectors;

/**
 * Created by ooee on 12/6/16.
 */
@Value
public class MultiplePrimitiveType implements ActualType {
    private List<PrimitiveType> primitiveTypes;

    @Override
    public String toCode() {
        List<String> primitiveTypesCode = primitiveTypes.stream().map(pt -> pt.toCode()).collect(Collectors.toList());
        return String.join(" ", primitiveTypesCode);
    }
}
