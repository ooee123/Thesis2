package ast;

import lombok.AllArgsConstructor;
import lombok.RequiredArgsConstructor;
import lombok.Value;

import java.util.Collections;
import java.util.List;

/**
 * Created by ooee on 9/24/16.
 */
@Value
@AllArgsConstructor
public class ParameterList {
    private final List<Parameter> parameters;

    public ParameterList() {
        parameters = Collections.emptyList();
    }
}
