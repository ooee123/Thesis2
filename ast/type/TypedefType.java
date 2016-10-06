package ast.type;

import ast.BaseElement;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.Value;

/**
 * Created by ooee on 9/24/16.
 */
@Data
public class TypedefType implements ActualType {
    private String typeName;

    public TypedefType(String typeName) {
        this.typeName = typeName;
    }

    @Override
    public String toCode() {
        return typeName;
    }
}
