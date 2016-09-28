package ast;

import lombok.Value;

import java.util.List;

/**
 * Created by ooee on 9/27/16.
 */
@Value
public class InitializerList implements AssignmentExpression {
    private List<AssignmentExpression> assignmentExpressions;
}
