package visitor;

import ast.BaseElement;
import lombok.Value;

/**
 * Created by ooee on 10/4/16.
 */
@Value
public class TypedBaseElement {
    private BaseElement baseElement;
    private TypeScope typeScope;
}
