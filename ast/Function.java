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
    private String originalLine;

    public Function(Type returnType, String identifier, ParameterList parameterList, CompoundStatement compoundStatement) {
        this(returnType, identifier, parameterList, compoundStatement, toCommentTip(returnType, identifier, parameterList));
    }

    @Override
    public String toCode(boolean showOriginalLine) {
        if (showOriginalLine) {
            return toCommentTip(returnType, identifier, parameterList) + " /* " + originalLine + " */" + compoundStatement.toCode(showOriginalLine);
        } else {
            return toCommentTip(returnType, identifier, parameterList) + compoundStatement.toCode(showOriginalLine);
        }
    }

    public static String toCommentTip(Type returnType, String identifier, ParameterList parameterList) {
        return returnType.toCode() + " " + identifier + "(" + parameterList.toCode() + ")";
    }
}
