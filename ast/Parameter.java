package ast;

import ast.expression.impl.PrimaryExpressionIdentifier;
import ast.type.PointerType;
import ast.type.Type;
import lombok.Data;
import lombok.EqualsAndHashCode;
import lombok.NonNull;

/**
 * Created by ooee on 9/24/16.
 */
@Data
@EqualsAndHashCode(callSuper = true)
public class Parameter extends AbstractParameter {
    @NonNull private PrimaryExpressionIdentifier formalParameterName;

    public Parameter(Type type, PrimaryExpressionIdentifier identifier) {
        super(type);
        this.formalParameterName = identifier;
    }

    @Override
    public String toCode() {
        if (super.getType() instanceof PointerType) {
            return super.getType().toCode() + formalParameterName.toCode();
        } else {
            return super.getType().toCode() + " " + formalParameterName.toCode();
        }
    }
}
