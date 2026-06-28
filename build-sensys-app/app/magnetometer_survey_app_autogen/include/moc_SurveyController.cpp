/****************************************************************************
** Meta object code from reading C++ file 'SurveyController.h'
**
** Created by: The Qt Meta Object Compiler version 69 (Qt 6.11.0)
**
** WARNING! All changes made in this file will be lost!
*****************************************************************************/

#include "../../../../magnetometer_survey_visualizer/app/include/SurveyController.h"
#include <QtCore/qmetatype.h>

#include <QtCore/qtmochelpers.h>

#include <memory>


#include <QtCore/qxptype_traits.h>
#if !defined(Q_MOC_OUTPUT_REVISION)
#error "The header file 'SurveyController.h' doesn't include <QObject>."
#elif Q_MOC_OUTPUT_REVISION != 69
#error "This file was generated using the moc from 6.11.0. It"
#error "cannot be used with the include files from this version of Qt."
#error "(The moc has changed too much.)"
#endif

#ifndef Q_CONSTINIT
#define Q_CONSTINIT
#endif

QT_WARNING_PUSH
QT_WARNING_DISABLE_DEPRECATED
QT_WARNING_DISABLE_GCC("-Wuseless-cast")
namespace {
struct qt_meta_tag_ZN16SurveyControllerE_t {};
} // unnamed namespace

template <> constexpr inline auto SurveyController::qt_create_metaobjectdata<qt_meta_tag_ZN16SurveyControllerE_t>()
{
    namespace QMC = QtMocConstants;
    QtMocHelpers::StringRefStorage qt_stringData {
        "SurveyController",
        "depthIndexChanged",
        "",
        "thresholdNtChanged",
        "processingChanged",
        "anomalyCountChanged",
        "progressChanged",
        "showAnomaliesChanged",
        "onProcessingComplete",
        "startSurvey",
        "name",
        "stopSurvey",
        "exportResults",
        "format",
        "path",
        "loadSession",
        "id",
        "depthIndex",
        "thresholdNt",
        "processing",
        "anomalyCount",
        "progress",
        "showAnomalies"
    };

    QtMocHelpers::UintData qt_methods {
        // Signal 'depthIndexChanged'
        QtMocHelpers::SignalData<void()>(1, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'thresholdNtChanged'
        QtMocHelpers::SignalData<void()>(3, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'processingChanged'
        QtMocHelpers::SignalData<void()>(4, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'anomalyCountChanged'
        QtMocHelpers::SignalData<void()>(5, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'progressChanged'
        QtMocHelpers::SignalData<void()>(6, 2, QMC::AccessPublic, QMetaType::Void),
        // Signal 'showAnomaliesChanged'
        QtMocHelpers::SignalData<void()>(7, 2, QMC::AccessPublic, QMetaType::Void),
        // Slot 'onProcessingComplete'
        QtMocHelpers::SlotData<void()>(8, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'startSurvey'
        QtMocHelpers::MethodData<void(const QString &)>(9, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 10 },
        }}),
        // Method 'stopSurvey'
        QtMocHelpers::MethodData<void()>(11, 2, QMC::AccessPublic, QMetaType::Void),
        // Method 'exportResults'
        QtMocHelpers::MethodData<void(const QString &, const QString &)>(12, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::QString, 13 }, { QMetaType::QString, 14 },
        }}),
        // Method 'loadSession'
        QtMocHelpers::MethodData<void(int)>(15, 2, QMC::AccessPublic, QMetaType::Void, {{
            { QMetaType::Int, 16 },
        }}),
    };
    QtMocHelpers::UintData qt_properties {
        // property 'depthIndex'
        QtMocHelpers::PropertyData<int>(17, QMetaType::Int, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 0),
        // property 'thresholdNt'
        QtMocHelpers::PropertyData<float>(18, QMetaType::Float, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 1),
        // property 'processing'
        QtMocHelpers::PropertyData<bool>(19, QMetaType::Bool, QMC::DefaultPropertyFlags, 2),
        // property 'anomalyCount'
        QtMocHelpers::PropertyData<int>(20, QMetaType::Int, QMC::DefaultPropertyFlags, 3),
        // property 'progress'
        QtMocHelpers::PropertyData<float>(21, QMetaType::Float, QMC::DefaultPropertyFlags, 4),
        // property 'showAnomalies'
        QtMocHelpers::PropertyData<bool>(22, QMetaType::Bool, QMC::DefaultPropertyFlags | QMC::Writable | QMC::StdCppSet, 5),
    };
    QtMocHelpers::UintData qt_enums {
    };
    return QtMocHelpers::metaObjectData<SurveyController, qt_meta_tag_ZN16SurveyControllerE_t>(QMC::MetaObjectFlag{}, qt_stringData,
            qt_methods, qt_properties, qt_enums);
}
Q_CONSTINIT const QMetaObject SurveyController::staticMetaObject = { {
    QMetaObject::SuperData::link<QObject::staticMetaObject>(),
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SurveyControllerE_t>.stringdata,
    qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SurveyControllerE_t>.data,
    qt_static_metacall,
    nullptr,
    qt_staticMetaObjectRelocatingContent<qt_meta_tag_ZN16SurveyControllerE_t>.metaTypes,
    nullptr
} };

void SurveyController::qt_static_metacall(QObject *_o, QMetaObject::Call _c, int _id, void **_a)
{
    auto *_t = static_cast<SurveyController *>(_o);
    if (_c == QMetaObject::InvokeMetaMethod) {
        switch (_id) {
        case 0: _t->depthIndexChanged(); break;
        case 1: _t->thresholdNtChanged(); break;
        case 2: _t->processingChanged(); break;
        case 3: _t->anomalyCountChanged(); break;
        case 4: _t->progressChanged(); break;
        case 5: _t->showAnomaliesChanged(); break;
        case 6: _t->onProcessingComplete(); break;
        case 7: _t->startSurvey((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1]))); break;
        case 8: _t->stopSurvey(); break;
        case 9: _t->exportResults((*reinterpret_cast<std::add_pointer_t<QString>>(_a[1])),(*reinterpret_cast<std::add_pointer_t<QString>>(_a[2]))); break;
        case 10: _t->loadSession((*reinterpret_cast<std::add_pointer_t<int>>(_a[1]))); break;
        default: ;
        }
    }
    if (_c == QMetaObject::IndexOfMethod) {
        if (QtMocHelpers::indexOfMethod<void (SurveyController::*)()>(_a, &SurveyController::depthIndexChanged, 0))
            return;
        if (QtMocHelpers::indexOfMethod<void (SurveyController::*)()>(_a, &SurveyController::thresholdNtChanged, 1))
            return;
        if (QtMocHelpers::indexOfMethod<void (SurveyController::*)()>(_a, &SurveyController::processingChanged, 2))
            return;
        if (QtMocHelpers::indexOfMethod<void (SurveyController::*)()>(_a, &SurveyController::anomalyCountChanged, 3))
            return;
        if (QtMocHelpers::indexOfMethod<void (SurveyController::*)()>(_a, &SurveyController::progressChanged, 4))
            return;
        if (QtMocHelpers::indexOfMethod<void (SurveyController::*)()>(_a, &SurveyController::showAnomaliesChanged, 5))
            return;
    }
    if (_c == QMetaObject::ReadProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: *reinterpret_cast<int*>(_v) = _t->depthIndex(); break;
        case 1: *reinterpret_cast<float*>(_v) = _t->thresholdNt(); break;
        case 2: *reinterpret_cast<bool*>(_v) = _t->processing(); break;
        case 3: *reinterpret_cast<int*>(_v) = _t->anomalyCount(); break;
        case 4: *reinterpret_cast<float*>(_v) = _t->progress(); break;
        case 5: *reinterpret_cast<bool*>(_v) = _t->showAnomalies(); break;
        default: break;
        }
    }
    if (_c == QMetaObject::WriteProperty) {
        void *_v = _a[0];
        switch (_id) {
        case 0: _t->setDepthIndex(*reinterpret_cast<int*>(_v)); break;
        case 1: _t->setThresholdNt(*reinterpret_cast<float*>(_v)); break;
        case 5: _t->setShowAnomalies(*reinterpret_cast<bool*>(_v)); break;
        default: break;
        }
    }
}

const QMetaObject *SurveyController::metaObject() const
{
    return QObject::d_ptr->metaObject ? QObject::d_ptr->dynamicMetaObject() : &staticMetaObject;
}

void *SurveyController::qt_metacast(const char *_clname)
{
    if (!_clname) return nullptr;
    if (!strcmp(_clname, qt_staticMetaObjectStaticContent<qt_meta_tag_ZN16SurveyControllerE_t>.strings))
        return static_cast<void*>(this);
    return QObject::qt_metacast(_clname);
}

int SurveyController::qt_metacall(QMetaObject::Call _c, int _id, void **_a)
{
    _id = QObject::qt_metacall(_c, _id, _a);
    if (_id < 0)
        return _id;
    if (_c == QMetaObject::InvokeMetaMethod) {
        if (_id < 11)
            qt_static_metacall(this, _c, _id, _a);
        _id -= 11;
    }
    if (_c == QMetaObject::RegisterMethodArgumentMetaType) {
        if (_id < 11)
            *reinterpret_cast<QMetaType *>(_a[0]) = QMetaType();
        _id -= 11;
    }
    if (_c == QMetaObject::ReadProperty || _c == QMetaObject::WriteProperty
            || _c == QMetaObject::ResetProperty || _c == QMetaObject::BindableProperty
            || _c == QMetaObject::RegisterPropertyMetaType) {
        qt_static_metacall(this, _c, _id, _a);
        _id -= 6;
    }
    return _id;
}

// SIGNAL 0
void SurveyController::depthIndexChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 0, nullptr);
}

// SIGNAL 1
void SurveyController::thresholdNtChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 1, nullptr);
}

// SIGNAL 2
void SurveyController::processingChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 2, nullptr);
}

// SIGNAL 3
void SurveyController::anomalyCountChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 3, nullptr);
}

// SIGNAL 4
void SurveyController::progressChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 4, nullptr);
}

// SIGNAL 5
void SurveyController::showAnomaliesChanged()
{
    QMetaObject::activate(this, &staticMetaObject, 5, nullptr);
}
QT_WARNING_POP
