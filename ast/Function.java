package ast;

import ast.type.Type;
import lombok.Value;

/**
 * Created by ooee on 9/24/16.
 */
@Value
public class Function implements BaseElement {
    private Type returnType;
    private String identifier;
    private ParameterList parameterList;
    private CompoundStatement compoundStatement;
}
