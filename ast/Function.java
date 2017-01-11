package ast;

import ast.statement.impl.CompoundStatement;
import ast.type.Type;
import lombok.AllArgsConstructor;
import lombok.Data;

/**
 * Created by ooee on 9/24/16.
 */
@Data
@AllArgsConstructor
public class Function implements BaseElement {
    private Type returnType;
    private String identifier;
    private ParameterList parameterList;
    private CompoundStatement compoundStatement;

    @Override
    public String toCode() {
        return returnType.toCode() + " " + identifier + "(" + parameterList.toCode() + ")" + compoundStatement.toCode();
    }
}
