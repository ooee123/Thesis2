package ast;

import ast.type.PointerType;
import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.Data;
import lombok.NonNull;
import lombok.Value;

/**
 * Created by ooee on 9/24/16.
 */
@Data
public class AbstractParameter implements BaseElement {
    @NonNull private Type type;

    @Override
    public String toCode() {
        return type.toCode();
    }
}
