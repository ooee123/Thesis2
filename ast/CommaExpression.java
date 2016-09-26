package ast;

import lombok.Value;

import java.util.List;

/**
 * Created by ooee on 9/26/16.
 */
@Value
public class CommaExpression implements Expression {
    private List<AssignmentExpression> assignmentExpressions;
}
